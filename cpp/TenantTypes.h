#pragma once

#include <cstdint>
#include <cstddef>
#include <atomic>
#include <array>
#include <vector>
#include <string>
#include <immintrin.h>

#pragma pack(push, 1)
struct Vector3 {
    float x, y, z;

    inline Vector3 operator-(const Vector3& b) const { return { x - b.x, y - b.y, z - b.z }; }
    inline Vector3 operator+(const Vector3& b) const { return { x + b.x, y + b.y, z + b.z }; }
    inline float Dot(const Vector3& b) const { return x * b.x + y * b.y + z * b.z; }
};

struct alignas(32) SimdVector4 {
    union {
        __m128 v;
        float f[4];
    };

    inline SimdVector4() : v(_mm_setzero_ps()) {}
    inline SimdVector4(__m128 m) : v(m) {}
    inline SimdVector4(float x, float y, float z, float w) : v(_mm_set_ps(w, z, y, x)) {}
};
#pragma pack(pop)

// Минимальное полное определение структуры AMX для сборки без сторонних SDK
struct AMX {
    uint8_t* base;
    uint8_t* data;
    int (*callback)(struct AMX* amx, int32_t index, int32_t* result, const int32_t* params);
    int (*debug)(struct AMX* amx);
    int32_t cip;
    int32_t frm;
    int32_t hea;
    int32_t hlw;
    int32_t stk;
    int32_t stp;
    int32_t flags;
    void* usermode;
    void* userdata[4];
    int32_t error;
    int32_t paramcount;
    int32_t pri;
    int32_t alt;
    int32_t reset_stk;
    int32_t reset_hea;
    int32_t sysreq_d;
};

template <typename T, size_t Capacity>
class LockFreeRingBuffer {
public:
    LockFreeRingBuffer() : head(0), tail(0) {}

    bool Push(const T& item) {
        size_t current_tail = tail.load(std::memory_order_relaxed);
        size_t next_tail = (current_tail + 1) % Capacity;
        if (next_tail == head.load(std::memory_order_acquire)) {
            return false;
        }
        buffer[current_tail] = item;
        tail.store(next_tail, std::memory_order_release);
        return true;
    }

    bool Pop(T& item) {
        size_t current_head = head.load(std::memory_order_relaxed);
        if (current_head == tail.load(std::memory_order_acquire)) {
            return false;
        }
        item = buffer[current_head];
        head.store((current_head + 1) % Capacity, std::memory_order_release);
        return true;
    }

private:
    alignas(64) std::array<T, Capacity> buffer;
    alignas(64) std::atomic<size_t> head;
    alignas(64) std::atomic<size_t> tail;
};

enum class TenantExecutionFlags : uint32_t {
    NONE = 0,
    ISOLATE_MEMORY = 1 << 0,
    JIT_ACCELERATED = 1 << 1,
    PROFILING_ENABLED = 1 << 2,
    STRICT_NETWORK = 1 << 3
};

inline TenantExecutionFlags operator|(TenantExecutionFlags a, TenantExecutionFlags b) {
    return static_cast<TenantExecutionFlags>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

inline bool operator&(TenantExecutionFlags a, TenantExecutionFlags b) {
    return (static_cast<uint32_t>(a) & static_cast<uint32_t>(b)) != 0;
}
