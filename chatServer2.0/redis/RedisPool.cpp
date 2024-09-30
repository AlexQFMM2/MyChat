#include "RedisPool.hpp"  

RedisPool::RedisPool(const std::string& host, int port, int pool_size)  
    : host(host), port(port), maxPoolSize(pool_size) {  
    for (int i = 0; i < maxPoolSize; ++i) {  
        redisContext* ctx = redisConnect(host.c_str(), port);  
        if (ctx == nullptr || ctx->err) {  
            if (ctx) {  
                std::cerr << "Error connecting to Redis: " << ctx->errstr << std::endl;  
                redisFree(ctx);  
            } else {  
                std::cerr << "Can't allocate Redis context" << std::endl;  
            }  
        } else {  
            connectionPool.push(ctx); // 将连接加入池中  
        }  
    }  
}  

RedisPool::~RedisPool() {  
    while (!connectionPool.empty()) {  
        redisFree(connectionPool.front()); // 释放连接  
        connectionPool.pop();  
    }  
}  

// 使用 unique_ptr 进行连接的获取  
std::unique_ptr<redisContext, std::function<void(redisContext*)>> RedisPool::getConnection() {  
    std::unique_lock<std::mutex> lock(mtx); // 加锁  
    cv.wait(lock, [this] { return !connectionPool.empty(); }); // 等待直到连接可用  

    redisContext* conn = connectionPool.front();  
    connectionPool.pop();  
    
    //std::cout << "pool can use thread : " << connectionPool.size() << std::endl;

    return std::unique_ptr<redisContext, std::function<void(redisContext*)>>(
        conn, 
        [this](redisContext* conn) { this->releaseConnection(conn); } // 使用 lambda 捕获 this 指针
    );
}  

void RedisPool::releaseConnection(redisContext* conn) {  
    if (conn) {  
        {  
            std::lock_guard<std::mutex> lock(mtx);  
            connectionPool.push(conn); // 将连接放回池中  
        }  
        //std::cout << "pool can use thread : " << connectionPool.size() << std::endl; 
        cv.notify_one(); // 通知等待的线程  
    }  
}