#pragma once

#include <chrono>
#include <string>
#include <unordered_map>
#include <mutex>

class ProfilerEngine {
public:
    static ProfilerEngine& GetInstance() {
        static ProfilerEngine instance;
        return instance;
    }

    void RecordMetric(const std::string& scope, uint64_t elapsed_nanos) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_aggregated_time[scope] += elapsed_nanos;
        m_call_counts[scope]++;
    }

    uint64_t GetAverageTimeNanos(const std::string& scope) {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_call_counts.find(scope);
        if (it == m_call_counts.end() || it->second == 0) return 0;
        return m_aggregated_time[scope] / it->second;
    }

private:
    std::mutex m_mutex;
    std::unordered_map<std::string, uint64_t> m_aggregated_time;
    std::unordered_map<std::string, uint64_t> m_call_counts;
};

class ScopedProfileTimer {
public:
    ScopedProfileTimer(std::string scope_name) 
        : m_scope(std::move(scope_name)), m_start(std::chrono::high_resolution_clock::now()) {}

    ~ScopedProfileTimer() {
        auto end = std::chrono::high_resolution_clock::now();
        uint64_t nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(end - m_start).count();
        ProfilerEngine::GetInstance().RecordMetric(m_scope, nanos);
    }

private:
    std::string m_scope;
    std::chrono::high_resolution_clock::time_point m_start;
};
