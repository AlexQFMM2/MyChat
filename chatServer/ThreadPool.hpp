#ifndef THREADPOOL_H
#define THREADPOOL_H

/*
工作线程（workers）：用于执行任务的线程。
任务队列（tasks）：任务放入这个队列中，线程会从中取出任务执行。
互斥锁（mutex）：保护任务队列的访问，确保多个线程不会同时修改任务队列。
条件变量（condition variable）：当任务队列有新任务加入时，通知工作线程去执行任务。
线程池状态：标记线程池是否还在运行。
*/

#include <vector>                 // 用于存储线程对象
#include <queue>                  // 任务队列
#include <thread>                 // 用于创建和管理线程
#include <mutex>                  // 互斥锁，用于保护共享数据
#include <condition_variable>     // 条件变量，用于线程同步
#include <functional>             // std::function 用于存储可调用对象
#include <future>                 // std::future 和 std::packaged_task 用于获取任务结果
#include <memory>                 // std::shared_ptr, std::make_shared 用于内存管理
#include <stdexcept>              // 用于异常处理

class ThreadPool {
public:
    // 构造函数：创建指定数量的工作线程
    ThreadPool(size_t numThreads);

    // 析构函数：确保线程池析构时，线程安全退出
    ~ThreadPool();

    // 向任务队列中添加任务，并返回该任务的future结果
    // 使用模板支持传入不同类型的任务
    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type>;
    //在模板函数中 && 不代表右值引用而是万能引用，即无论左值右值
    //future:是 C++11 提供的一种机制，用于异步任务的返回值
    //std::result_of<F(Args...)>::type 是一种类型推导机制，它用来推导可调用对象 F 使用参数 Args... 时的返回类型

private:
    std::vector<std::thread> workers; // 工作线程列表
    std::queue<std::function<void()>> tasks; // 任务队列

    std::mutex queueMutex; // 保护任务队列的互斥锁
    std::condition_variable condition; // 条件变量，通知线程有新任务
    bool stop; // 标志线程池是否停止接收新任务
};

// 任务入队函数的实现
// enqueue 函数的作用是将一个任务（函数对象）加入任务队列，
// 并返回该任务的 future，用于获取任务的返回值。
template<class F, class... Args>
auto ThreadPool::enqueue(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type> {
    // 确定任务的返回值类型
    using return_type = typename std::result_of<F(Args...)>::type;

    // 将任务包裹成 std::packaged_task，这样可以通过 future 获取任务的返回值
    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );

    // 获取任务的 future，以便在任务完成后可以获取结果
    std::future<return_type> res = task->get_future();

    {
        // 互斥锁，保护任务队列的访问，避免多个线程同时修改队列
        std::unique_lock<std::mutex> lock(queueMutex);

        // 如果线程池已停止，则抛出异常，不能再添加新任务
        if (stop)
            throw std::runtime_error("enqueue on stopped ThreadPool");

        // 将任务加入任务队列
        tasks.emplace([task]() { (*task)(); });
    }
    
    // 通知一个等待的工作线程去执行任务
    condition.notify_one();
    return res; // 返回 future 对象，供调用者获取任务执行结果
}

#endif // THREADPOOL_H