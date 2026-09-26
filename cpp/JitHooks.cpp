#include "JitHooks.h"
#include <cstring>

#if defined(__linux__) || defined(__APPLE__)
#include <sys/mman.h>
#include <unistd.h>
#else
#include <windows.h>
#endif

namespace JitCore {

    bool SetMemoryWritable(void* address, size_t size, bool writable) {
#if defined(__linux__) || defined(__APPLE__)
        size_t page_size = sysconf(_SC_PAGESIZE);
        uintptr_t addr = reinterpret_cast<uintptr_t>(address);
        uintptr_t page_start = addr & ~(page_size - 1);
        int prot = PROT_READ | PROT_EXEC | (writable ? PROT_WRITE : 0);
        return mprotect(reinterpret_cast<void*>(page_start), (addr + size) - page_start, prot) == 0;
#else
        DWORD old_protect;
        DWORD new_protect = writable ? PAGE_EXECUTE_READWRITE : PAGE_EXECUTE_READ;
        return VirtualProtect(address, size, new_protect, &old_protect) != 0;
#endif
    }

    bool InstallDetour(HookContext* ctx, void* target, void* detour, size_t size) {
        if (!ctx || !target || !detour || size < 5) return false;

        ctx->target_address = target;
        ctx->detour_address = detour;
        ctx->patch_size = size;
        ctx->is_installed = false;

        std::memcpy(ctx->original_bytes, target, size);

        if (!SetMemoryWritable(target, size, true)) return false;

        uint8_t* patch = reinterpret_cast<uint8_t*>(target);
        patch[0] = 0xE9;

        uintptr_t rel_offset = reinterpret_cast<uintptr_t>(detour) - (reinterpret_cast<uintptr_t>(target) + 5);
        std::memcpy(&patch[1], &rel_offset, sizeof(uint32_t));

        for (size_t i = 5; i < size; ++i) {
            patch[i] = 0x90;
        }

        SetMemoryWritable(target, size, false);
        ctx->is_installed = true;
        return true;
    }

    bool RemoveDetour(HookContext* ctx) {
        if (!ctx || !ctx->is_installed) return false;

        if (!SetMemoryWritable(ctx->target_address, ctx->patch_size, true)) return false;

        std::memcpy(ctx->target_address, ctx->original_bytes, ctx->patch_size);
        SetMemoryWritable(ctx->target_address, ctx->patch_size, false);

        ctx->is_installed = false;
        return true;
    }

}
