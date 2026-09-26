#if defined _tenant_virtualizer_included
    #endinput
#endif
#define _tenant_virtualizer_included

enum (<<= 1) {
    TENANT_FLAG_NONE = 0,
    TENANT_FLAG_ISOLATE_MEMORY = 1, 
    TENANT_FLAG_JIT_ACCELERATED,    
    TENANT_FLAG_PROFILING_ENABLED,   
    TENANT_FLAG_STRICT_NETWORK       
};

native Tenant_CreateInstance(const script_path[], flags = TENANT_FLAG_ISOLATE_MEMORY);
native Tenant_DestroyInstance(tenant_id);
native Tenant_Bind(player_id, tenant_id);
native Tenant_Unbind(player_id);
native Tenant_GetScopeAverageNanos(const scope_name[]);
