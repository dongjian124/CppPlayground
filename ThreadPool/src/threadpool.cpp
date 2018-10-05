//
// Created by sb on 18-10-5.
//

#include <future>
#include "threadpool.h"

ThreadPool::ThreadPool(int thread_count) : stop_(false)
{
    for (size_t i = 0; i < thread_count; ++i)
    {
        threads_.emplace_back([this]
                              {
                                  for (;;) //无线循环,保持线程在线
                                  {
                                      Task task;
                                      {
                                          std::unique_lock<std::mutex> lock(mutex_);
                                          cond_.wait(lock, [this] { return stop_ || !tasks_.empty(); });  //等待任务到来或者终止
                                          if (stop_)
                                              return;
                                          task = std::move(tasks_.front());
                                          tasks_.pop();
                                      }
                                      task();
                                  }
                              });
    }
}

ThreadPool::~ThreadPool()
{
    {
        std::unique_lock<std::mutex> lock(mutex_);
        stop_ = true;
    }
    cond_.notify_all();
    for (auto &th : threads_)
        th.join();
}

