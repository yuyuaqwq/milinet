#pragma once

#include <cstdint>
#include <chrono>
#include <functional>
#include <queue>
#include <vector>
#include <mutex>
#include <algorithm>

#include "milinet/noncopyable.h"

namespace milinet {

namespace detail {

class MinHeapTimer : noncopyable {
public:
    struct Task {
        using Func = std::function<void(const Task&)>;

        Task(std::chrono::high_resolution_clock::time_point expiration, const Func& func)
            : expiration(expiration)
            , func(func) {}

        bool operator>(const Task& other) const {
            return expiration > other.expiration;
        }

        std::chrono::high_resolution_clock::time_point expiration;
        Func func;
    };

    using TaskQueue = std::priority_queue<Task, std::vector<Task>, std::greater<Task>>;

public:
    MinHeapTimer() = default;
    ~MinHeapTimer() = default;

    void AddTaskWithGuard(size_t tick, const Task::Func& func) {
        tasks_.emplace_back(tick, func);
    }

    // �������ָ���ڵ�ǰʱ��� delay_ms ����ִ��
    void AddTask(size_t delay_ms, const Task::Func& func) {
        auto expiration = std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(delay_ms);
        std::lock_guard<std::mutex> guard(mutex_);
        tasks_.emplace(expiration, func);
    }

    // ��ʱ��ÿ�ε��ü���������ִ��
    void Tick() {
        std::lock_guard<std::mutex> guard(mutex_);
        auto now = std::chrono::high_resolution_clock::now();

        // ѭ�����Ѷ������Ƿ���
        while (!tasks_.empty() && tasks_.top().expiration <= now) {
            auto& task = tasks_.top();
            task.func(task);  // ִ������
            tasks_.pop();
        }
    }

    // ������һ�������ʣ��ʱ�䣬�����Ż��ȴ�ʱ��
    size_t GetNextTimeoutMs() const {
        std::lock_guard<std::mutex> guard(mutex_);
        if (tasks_.empty()) {
            return static_cast<size_t>(-1); // ������
        }

        auto now = std::chrono::steady_clock::now();
        auto next_task = tasks_.top();
        auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(next_task.expiration - now).count();
        return diff > 0 ? static_cast<size_t>(diff) : 0;
    }

private:
    mutable std::mutex mutex_;
    TaskQueue tasks_;  // ��С�ѣ�������ʱ�������������У�
};

} // namespace detail

} // namespace milinet
