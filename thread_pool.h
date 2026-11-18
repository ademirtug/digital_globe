#pragma once

#include <atomic>
#include <condition_variable>
#include <future>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>
#include <stdexcept>
#include <stop_token>

class thread_pool {
public:
    explicit thread_pool(size_t max_threads = std::thread::hardware_concurrency()) {
        start(max_threads);
    }

    ~thread_pool() {
        shutdown();
    }

    thread_pool(const thread_pool&) = delete;
    thread_pool& operator=(const thread_pool&) = delete;

    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args) -> std::future<std::invoke_result_t<F, Args...>> {
        using return_type = std::invoke_result_t<F, Args...>;

        auto task = std::make_shared<std::packaged_task<return_type()>>(
            [func = std::forward<F>(f), ...args = std::forward<Args>(args)]() mutable {
                return func(std::move(args)...);
            }
        );

        std::future<return_type> result = task->get_future();
        {
            std::scoped_lock lock(queue_mutex_);
            if (stop_requested_.load(std::memory_order_acquire)) {
                throw std::runtime_error("enqueue on stopped thread_pool");
            }
            tasks_.emplace([task]() { (*task)(); });
        }

        cv_.notify_one();
        return result;
    }

    void shutdown() {
        stop_requested_.store(true, std::memory_order_release);
        cv_.notify_all();

        threads_.clear();
    }

    void resize(size_t new_size) {
        shutdown();
        start(new_size);
    }

private:
    void start(size_t thread_count) {
        stop_requested_.store(false, std::memory_order_release);
        for (size_t i = 0; i < thread_count; ++i) {
            threads_.emplace_back([this](std::stop_token stoken) { worker_loop(stoken); });
        }
    }

    void worker_loop(std::stop_token stoken) {
        while (!stoken.stop_requested()) {
            std::function<void()> task;
            {
                std::unique_lock lock(queue_mutex_);
                cv_.wait(lock, [this, &stoken] {
                    return stop_requested_.load(std::memory_order_acquire) || !tasks_.empty() || stoken.stop_requested();
                    });

                if ((stop_requested_.load(std::memory_order_acquire) && tasks_.empty()) || stoken.stop_requested())
                    return;

                if (!tasks_.empty()) {
                    task = std::move(tasks_.front());
                    tasks_.pop();
                }
                else {
                    continue;
                }
            }

            task();
        }
    }

    std::vector<std::jthread> threads_;
    std::queue<std::function<void()>> tasks_;
    std::mutex queue_mutex_;
    std::condition_variable cv_;
    std::atomic<bool> stop_requested_{ false };
};


