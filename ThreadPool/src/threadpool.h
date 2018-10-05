//
// Created by sb on 18-10-5.
//

#ifndef PROJECT_THREADPOOL_H
#define PROJECT_THREADPOOL_H

#include <thread>
#include <vector>
#include <functional>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <future>

class ThreadPool
{
public:

    using Task = std::function<void()>;

    explicit ThreadPool(int);

    template<class F, class ... Args>
    auto AddTask(F &&f, Args &&... args) -> std::future<std::result_of_t<F(Args...)>>
    {
        using return_type = std::result_of_t<F(Args...)>;

        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );

        auto ret = task->get_future();
        {
            std::unique_lock<std::mutex> lock(mutex_);
            if (stop_)
                throw std::runtime_error("add task on stopped thread pool");
            tasks_.emplace([task] { (*task)(); });
        }
        cond_.notify_one();
        return ret;
    }

    ~ThreadPool();

private:
    std::vector<std::thread> threads_;
    int thread_count_;
    std::queue<Task> tasks_;

    std::mutex mutex_;
    std::condition_variable cond_;
    bool stop_;
};

#endif //PROJECT_THREADPOOL_H
