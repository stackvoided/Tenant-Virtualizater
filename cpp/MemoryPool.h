#pragma once

#include <cstddef>
#include <cstdint>
#include <mutex>
#include <vector>

#if defined(__linux__) || defined(__APPLE__)
#include <sys/mman.h>
#include <unistd.h>
#else
#include <windows.h>
#endif

class PageAlignedAllocator {
public:
    static void* AllocateExecutable(size_t bytes) {
        size_t page_size = GetPageSize();
        size_t aligned_size = (bytes + page_size - 1) & ~(page_size - 1);
#if defined(__linux__) || defined(__APPLE__)
        void* ptr = mmap(NULL, aligned_size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        return (ptr == MAP_FAILED) ? nullptr : ptr;
#else
        return VirtualAlloc(NULL, aligned_size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
#endif
    }

    static void FreeExecutable(void* ptr, size_t bytes) {
        if (!ptr) return;
#if defined(__linux__) || defined(__APPLE__)
        size_t page_size = GetPageSize();
        size_t aligned_size = (bytes + page_size - 1) & ~(page_size - 1);
        munmap(ptr, aligned_size);
#else
        (void)bytes;
        VirtualFree(ptr, 0, MEM_RELEASE);
#endif
    }

    static size_t GetPageSize() {
#if defined(__linux__) || defined(__APPLE__)
        return static_cast<size_t>(sysconf(_SC_PAGESIZE));
#else
        SYSTEM_INFO si;
        GetSystemInfo(&si);
        return static_cast<size_t>(si.dwPageSize);
#endif
    }
};

class AmxSlabPool {
public:
    AmxSlabPool(size_t chunk_size, size_t initial_chunks) 
        : m_chunk_size(chunk_size) {
        Grow(initial_chunks);
    }

    ~AmxSlabPool() {
        for (void* ptr : m_pages) {
            PageAlignedAllocator::FreeExecutable(ptr, m_chunk_size);
        }
    }

    void* Acquire() {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_free_list.empty()) {
            Grow(8);
        }
        void* ptr = m_free_list.back();
        m_free_list.pop_back();
        return ptr;
    }

    void Release(void* ptr) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_free_list.push_back(ptr);
    }

private:
    void Grow(size_t count) {
        for (size_t i = 0; i < count; ++i) {
            void* ptr = PageAlignedAllocator::AllocateExecutable(m_chunk_size);
            if (ptr) {
                m_pages.push_back(ptr);
                m_free_list.push_back(ptr);
            }
        }
    }

    size_t m_chunk_size;
    std::mutex m_mutex;
    std::vector<void*> m_pages;
    std::vector<void*> m_free_list;
};
