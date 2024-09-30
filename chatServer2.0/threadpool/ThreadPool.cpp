#include "ThreadPool.hpp"

// 构造函数
ThreadPool::ThreadPool(size_t numThreads) : stop(false) {
    // 创建 numThreads 个线程
    for (size_t i = 0; i < numThreads; ++i) {
        // 使用 emplace_back 向 workers 容器中添加线程
        // lambda 表达式定义了每个线程的工作内容
        // this 捕获成员变量，让下面的大括号能使用成员函数
        workers.emplace_back([this] {
            // 无限循环，线程不断等待和执行任务
            for (;;) {
                std::function<void()> task;

                {
                    // 加锁，保护任务队列的访问
                    std::unique_lock<std::mutex> lock(this->queueMutex);

                    // 使用条件变量等待任务的到来，或者等待线程池的 stop 状态
                    // 第二个 lambda 表达式用来判断是否应该停止等待
                    this->condition.wait(lock, [this] {
                        // 线程在以下两种情况下会被唤醒：
                        // 1. stop 被设置为 true，表示线程池正在停止。
                        // 2. 任务队列中有新的任务到来（tasks 非空）。
                        return this->stop || !this->tasks.empty();
                    });

                    // 如果线程池停止了并且任务队列是空的，那么当前线程退出
                    if (this->stop && this->tasks.empty())
                        return;

                    // 从任务队列中取出一个任务
                    task = std::move(this->tasks.front());
                    this->tasks.pop(); // 移除已取出的任务
                }

                // 执行任务
                task();
            }
        });
    }
}


// 析构函数
ThreadPool::~ThreadPool() {
    
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        stop = true;
    }

    condition.notify_all();
    for (std::thread &worker : workers)
        worker.join();
}
