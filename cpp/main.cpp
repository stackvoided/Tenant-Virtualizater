#include "TenantManager.h"
#include "Profiler.h"
#include <cstdint>

#if defined(WIN32) || defined(_WIN32)
    #define EXPORT_API __declspec(dllexport)
#else
    #define EXPORT_API __attribute__((visibility("default")))
#endif

extern "C" {

EXPORT_API uint32_t Supports() {
    return 0x0200 | 0x10000;
}

EXPORT_API bool Load(void** ppData) {
    (void)ppData;
    return TenantManager::GetInstance().Initialize(64);
}

EXPORT_API void Unload() {
    TenantManager::GetInstance().Shutdown();
}

EXPORT_API int32_t Tenant_CreateInstance(const char* path, uint32_t flags) {
    if (!path) return -1;
    return TenantManager::GetInstance().AllocateTenant(path, static_cast<TenantExecutionFlags>(flags));
}

EXPORT_API int32_t Tenant_DestroyInstance(int32_t id) {
    return TenantManager::GetInstance().DeallocateTenant(id) ? 1 : 0;
}

EXPORT_API int32_t Tenant_Bind(int32_t player_id, int32_t tenant_id) {
    return TenantManager::GetInstance().AttachPlayer(player_id, tenant_id) ? 1 : 0;
}

EXPORT_API int32_t Tenant_Unbind(int32_t player_id) {
    return TenantManager::GetInstance().DetachPlayer(player_id) ? 1 : 0;
}

EXPORT_API uint64_t Tenant_GetScopeAverageNanos(const char* scope) {
    if (!scope) return 0;
    return ProfilerEngine::GetInstance().GetAverageTimeNanos(scope);
}

}
