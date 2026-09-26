#include "TenantManager.h"
#include "Profiler.h"
#include <fstream>
#include <cstring>
#include <algorithm>

TenantManager& TenantManager::GetInstance() {
    static TenantManager instance;
    return instance;
}

TenantManager::TenantManager() {}
TenantManager::~TenantManager() { Shutdown(); }

bool TenantManager::Initialize(size_t max_tenants) {
    std::lock_guard<std::mutex> lock(m_global_lock);
    m_memory_pool = std::make_unique<AmxSlabPool>(1024 * 1024 * 4, max_tenants); // 4MB per tenant
    return true;
}

void TenantManager::Shutdown() {
    std::lock_guard<std::mutex> lock(m_global_lock);
    for (auto& [id, tenant] : m_tenants) {
        if (tenant->memory_buffer) {
            m_memory_pool->Release(tenant->memory_buffer);
        }
        delete tenant->amx;
    }
    m_tenants.clear();
    m_player_route_table.clear();
}

int32_t TenantManager::AllocateTenant(const std::string& amx_path, TenantExecutionFlags flags) {
    ScopedProfileTimer timer("TenantManager::AllocateTenant");

    std::ifstream file(amx_path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) return -1;

    size_t file_size = static_cast<size_t>(file.tellg());
    file.seekg(0, std::ios::beg);

    void* chunk = m_memory_pool->Acquire();
    if (!chunk) return -1;

    if (!file.read(reinterpret_cast<char*>(chunk), file_size)) {
        m_memory_pool->Release(chunk);
        return -1;
    }

    int32_t id = m_id_generator.fetch_add(1, std::memory_order_relaxed);
    auto instance = std::make_unique<TenantInstance>();
    instance->id = id;
    instance->script_path = amx_path;
    instance->active.store(true, std::memory_order_release);
    instance->flags = flags;
    instance->memory_buffer = chunk;
    instance->memory_size = file_size;
    instance->amx = new AMX();

    std::memset(instance->amx, 0, sizeof(AMX));
    instance->amx->base = reinterpret_cast<uint8_t*>(chunk);

    {
        std::lock_guard<std::mutex> lock(m_global_lock);
        m_tenants[id] = std::move(instance);
    }

    return id;
}

bool TenantManager::DeallocateTenant(int32_t tenant_id) {
    ScopedProfileTimer timer("TenantManager::DeallocateTenant");
    std::lock_guard<std::mutex> lock(m_global_lock);

    auto it = m_tenants.find(tenant_id);
    if (it == m_tenants.end()) return false;

    TenantInstance* inst = it->second.get();
    inst->active.store(false, std::memory_order_release);

    {
        std::lock_guard<std::mutex> t_lock(inst->tenant_mutex);
        for (int32_t pid : inst->connected_players) {
            m_player_route_table.erase(pid);
        }
        inst->connected_players.clear();

        if (inst->memory_buffer) {
            m_memory_pool->Release(inst->memory_buffer);
            inst->memory_buffer = nullptr;
        }
        delete inst->amx;
        inst->amx = nullptr;
    }

    m_tenants.erase(it);
    return true;
}

bool TenantManager::AttachPlayer(int32_t player_id, int32_t tenant_id) {
    std::lock_guard<std::mutex> lock(m_global_lock);
    auto it = m_tenants.find(tenant_id);
    if (it == m_tenants.end()) return false;

    m_player_route_table[player_id] = tenant_id;

    std::lock_guard<std::mutex> t_lock(it->second->tenant_mutex);
    it->second->connected_players.push_back(player_id);
    return true;
}

bool TenantManager::DetachPlayer(int32_t player_id) {
    std::lock_guard<std::mutex> lock(m_global_lock);
    auto route_it = m_player_route_table.find(player_id);
    if (route_it == m_player_route_table.end()) return false;

    int32_t tenant_id = route_it->second;
    m_player_route_table.erase(route_it);

    auto t_it = m_tenants.find(tenant_id);
    if (t_it != m_tenants.end()) {
        std::lock_guard<std::mutex> t_lock(t_it->second->tenant_mutex);
        auto& players = t_it->second->connected_players;
        players.erase(std::remove(players.begin(), players.end(), player_id), players.end());
    }

    return true;
}

int32_t TenantManager::ResolvePlayerTenant(int32_t player_id) {
    std::lock_guard<std::mutex> lock(m_global_lock);
    auto it = m_player_route_table.find(player_id);
    return (it != m_player_route_table.end()) ? it->second : -1;
}

TenantInstance* TenantManager::GetTenantContext(int32_t tenant_id) {
    std::lock_guard<std::mutex> lock(m_global_lock);
    auto it = m_tenants.find(tenant_id);
    return (it != m_tenants.end()) ? it->second.get() : nullptr;
}

int TenantManager::ExecutePublic(int32_t tenant_id, const char* name, int32_t* retval) {
    ScopedProfileTimer timer("TenantManager::ExecutePublic");
    TenantInstance* inst = GetTenantContext(tenant_id);
    if (!inst || !inst->active.load(std::memory_order_acquire) || !name) return 1;

    std::lock_guard<std::mutex> t_lock(inst->tenant_mutex);
    if (retval) *retval = 0;
    return 0;
}
