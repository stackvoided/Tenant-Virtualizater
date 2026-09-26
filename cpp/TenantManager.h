#pragma once

#include "TenantTypes.h"
#include "MemoryPool.h"
#include <unordered_map>
#include <memory>
#include <mutex>
#include <atomic>

struct TenantInstance {
    uint32_t id;
    std::string script_path;
    std::atomic<bool> active;
    TenantExecutionFlags flags;
    AMX* amx;
    void* memory_buffer;
    size_t memory_size;
    std::vector<int32_t> connected_players;
    alignas(64) std::mutex tenant_mutex;
};

class TenantManager {
public:
    static TenantManager& GetInstance();

    bool Initialize(size_t max_tenants);
    void Shutdown();

    int32_t AllocateTenant(const std::string& amx_path, TenantExecutionFlags flags);
    bool DeallocateTenant(int32_t tenant_id);

    bool AttachPlayer(int32_t player_id, int32_t tenant_id);
    bool DetachPlayer(int32_t player_id);
    int32_t ResolvePlayerTenant(int32_t player_id);

    int ExecutePublic(int32_t tenant_id, const char* name, int32_t* retval);

    TenantInstance* GetTenantContext(int32_t tenant_id);

private:
    TenantManager();
    ~TenantManager();

    std::atomic<uint32_t> m_id_generator{ 1 };
    mutable std::mutex m_global_lock;
    std::unique_ptr<AmxSlabPool> m_memory_pool;
    std::unordered_map<int32_t, std::unique_ptr<TenantInstance>> m_tenants;
    std::unordered_map<int32_t, int32_t> m_player_route_table;
};
