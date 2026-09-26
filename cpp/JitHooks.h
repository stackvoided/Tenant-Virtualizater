#pragma once

#include <cstdint>
#include <cstddef>

namespace JitCore {

    enum class HookType {
        DETOUR_JMP_X86,
        DETOUR_JMP_X64,
        VTABLE_SWAP
    };

    struct HookContext {
        void* target_address;
        void* detour_address;
        uint8_t original_bytes[14];
        size_t patch_size;
        bool is_installed;
    };

    bool InstallDetour(HookContext* ctx, void* target, void* detour, size_t size);
    bool RemoveDetour(HookContext* ctx);
    bool SetMemoryWritable(void* address, size_t size, bool writable);

}
