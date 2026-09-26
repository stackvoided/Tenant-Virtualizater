<div align="center">

# AMX Multi-Tenant Sandbox Engine (`v2.0 Enterprise`)

**Zero-overhead virtualized execution context manager, SIMD-accelerated spatial engine, and isolated memory pool for SA-MP / open.mp server environments.**

[![C++ Standard](https://img.shields.io/badge/C%2B%2B-20-00599C.svg?style=flat-square&logo=c%2B%2B)](https://en.cppreference.com/w/cpp/20)
[![Target Architecture](https://img.shields.io/badge/Arch-x86%20%2F%20i686-red.svg?style=flat-square)](#-compilation--toolchain)
[![Platforms](https://img.shields.io/badge/Platform-Linux%20%7C%20Windows-lightgrey.svg?style=flat-square)](#-compilation--toolchain)
[![License](https://img.shields.io/badge/License-MIT-blue.svg?style=flat-square)](LICENSE)

</div>

---

## Architecture Overview 📐

**AMX Multi-Tenant Sandbox Engine** is an enterprise-grade C++20 dynamic shared object designed to solve execution thread contention and memory isolation bottlenecks in standard SA-MP / open.mp runtime environments. By implementing isolated virtualized instances ("tenants"), the engine allows multiple autonomous `.amx` bytecodes to execute concurrently within a single process space without state corruption or main-thread frame degradation.

```text
┌──────────────────────────────────────────────┐
│              Main Server Process Space       │
└──────────────────────┬───────────────────────┘
                       │
┌──────────────────────▼───────────────────────┐
│       Zero-Lock Network Router (RingBuffer)  │
└──────────────┬───────────────────────┬───────┘
               │                       │
     ┌─────────▼─────────┐   ┌─────────▼─────────┐
     │ Tenant Instance #1│   │ Tenant Instance #2│
     │       (Slab)      │   │       (Slab)      │
     ├───────────────────┤   ├───────────────────┤
     │ • AVX2 Raycast SIMD│   │ • AVX2 Raycast SIMD│
     │ • Memory Protection│   │ • Memory Protection│
     │ • Real-Time Profiler│  │ • Real-Time Profiler│
     └───────────────────┘   └───────────────────┘
```

### Key Engineering Features

- **Memory-Isolated Multi-Tenancy:** Enables dynamic creation and tearing down of isolated execution sandboxes for independent minigames, sub-servers, or distinct virtual worlds without global server-side effects or crashes.
- **Page-Aligned Slab Allocator:** Custom `mmap` / `VirtualAlloc` memory pool utilizing aligned arena allocation strategies to bypass OS kernel runtime overhead during rapid AMX script dynamic instantiation and destruction.
- **Lock-Free Pipeline Network Routing:** Atomic, lock-free ring buffer architecture (`LockFreeRingBuffer`) designed for ultra-low latency packet dispatching and client-to-tenant routing under heavy payload concurrent strain.
- **AVX2 / SIMD Vector Mathematics Engine:** Hardware-accelerated Ray-OBB/AABB bounding box collision evaluation (`RayOBBIntersectionAVX2`) executing concurrent floating-point transformations across 128/256-bit registers.
- **Low-Level Inline JIT Detouring:** Custom x86 memory patcher providing safe runtime hook installation, instruction detouring, and RWX memory protection swapping.
- **Nanosecond Performance Profiler:** High-resolution internal telemetry engine measuring execution scope overhead with zero-cost RAII timer abstractions.

---

## Codebase Structure 🗂

```text
├── AmxHooks.cpp              # Low-level AMX runtime callback interception
├── JitHooks.cpp              # VirtualMemory protection & x86 detour engine
├── JitHooks.h                # Detour engine definitions and executable flags
├── MemoryPool.h               # Slab/Arena allocator with page-aligned execution bounds
├── NetworkRouter.cpp          # Lock-free atomic ring buffer network router
├── Profiler.h                 # High-resolution nanosecond execution scope profiler
├── SkeletalRaycast.cpp        # AVX2/SIMD spatial vector mathematics and raycasting
├── TenantManager.cpp          # Lifecycle orchestrator for virtual tenant states
├── TenantManager.h            # Tenant state containers and thread-safe registries
├── TenantTypes.h              # Memory structures, SIMD primitives, lock-free buffers
├── main.cpp                   # C-ABI export bindings and SA-MP plugin entry points
├── Makefile                   # Cross-platform 32-bit (i686) toolchain configuration

## 💻 Toolchain Requirements

- **Target Architecture:** x86 / i686 (32-bit mandatory for SA-MP / open.mp process compatibility).
- **Linux Toolchain:** `g++` (C++20 capable), `g++-multilib`, `gcc-multilib`, `make`.
- **Windows Toolchain (Cross-Compilation):** `i686-w64-mingw32-g++` (via mingw-w64).

---

## ⚙️ Compilation & Build

The build system utilizes a zero-dependency, optimized multi-architecture Makefile.

### 1. Host Dependencies Setup (Debian / Ubuntu)

```bash
sudo apt update
sudo apt install -y build-essential g++-multilib gcc-multilib mingw-w64
```

### 2. Building for Linux Target (`TenantVirtualizer.so`)

```bash
make linux
```

### 3. Building for Windows Target (`TenantVirtualizer.dll`)

```bash
make windows
```

### 4. Cleaning Build Artifacts

```bash
make clean
```

---

## 🚀 Deployment & Integration

### 1. Server Configuration

Copy the compiled shared object into your server's `plugins/` directory and append it to your runtime `server.cfg`.

**Linux deployment:**

```text
plugins TenantVirtualizer.so
```

**Windows deployment:**

```text
plugins TenantVirtualizer.dll
```

### 2. PAWN Include Integration

Copy `tenant_virtualizer.inc` to your `pawno/include/` directory and include it within your project source:

```pawn
#include <a_samp>
#include <tenant_virtualizer>
```

---

## 📖 Production Usage Example

```pawn
#include <a_samp>
#include <tenant_virtualizer>

static g_PaintballTenant = -1;

public OnGameModeInit()
{
    // Instantiate an isolated runtime context with memory isolation and real-time profiling
    g_PaintballTenant = Tenant_CreateInstance(
        "scriptfiles/tenants/paintball_logic.amx",
        TENANT_FLAG_ISOLATE_MEMORY | TENANT_FLAG_PROFILING_ENABLED
    );

    if (g_PaintballTenant != -1)
    {
        printf("[Sandbox Engine] Successfully instantiated tenant ID: %d", g_PaintballTenant);
    }

    return 1;
}

public OnGameModeExit()
{
    // Graceful teardown and memory release
    if (g_PaintballTenant != -1)
    {
        Tenant_DestroyInstance(g_PaintballTenant);
    }

    return 1;
}

public OnPlayerCommandText(playerid, cmdtext[])
{
    if (!strcmp(cmdtext, "/joinpaintball", true))
    {
        if (g_PaintballTenant == -1)
            return SendClientMessage(playerid, -1, "Tenant instance offline.");

        // Bind network routing and execution context to the tenant space
        Tenant_Bind(playerid, g_PaintballTenant);
        SetPlayerVirtualWorld(playerid, 101);
        SendClientMessage(playerid, -1, "Routed to Paintball Sandbox Environment.");

        return 1;
    }

    if (!strcmp(cmdtext, "/leavepaintball", true))
    {
        // Unbind player context and route back to global execution space
        Tenant_Unbind(playerid);
        SetPlayerVirtualWorld(playerid, 0);
        SendClientMessage(playerid, -1, "Routed back to Global Master Sandbox.");

        return 1;
    }

    return 0;
}
```

---

## 📊 PAWN C-ABI API Reference

| Native Signature | Description | Return Value |
|---|---|---|
| `Tenant_CreateInstance(const path[], flags)` | Allocates aligned memory and instantiates an isolated AMX tenant context. | `tenant_id` or `-1` on error |
| `Tenant_DestroyInstance(tenant_id)` | Tears down a tenant instance and releases allocated Slab memory pages. | `1` on success, `0` on error |
| `Tenant_Bind(player_id, tenant_id)` | Binds client network routing and execution context to the target tenant. | `1` on success, `0` on error |
| `Tenant_Unbind(player_id)` | Unbinds client context and returns routing to the global fallback. | `1` on success, `0` on error |
| `Tenant_GetScopeAverageNanos(const scope[])` | Queries execution telemetry for named profiling scopes. | Scope time in nanoseconds (`uint64`) |

### Execution Bitmask Flags (`TenantExecutionFlags`)

| Flag | Value | Description |
|---|---:|---|
| `TENANT_FLAG_NONE` | `0x00` | Default standard execution context. |
| `TENANT_FLAG_ISOLATE_MEMORY` | `0x01` | Strict page-aligned memory isolation. |
| `TENANT_FLAG_JIT_ACCELERATED` | `0x02` | Enables JIT detouring and patch optimization. |
| `TENANT_FLAG_PROFILING_ENABLED` | `0x04` | Enables high-resolution nanosecond telemetry logging. |
| `TENANT_FLAG_STRICT_NETWORK` | `0x08` | Enables lock-free pipeline ring buffer routing. |

---

## 📜 License

Distributed under the **MIT License**. See [`LICENSE`](LICENSE) for further details.
