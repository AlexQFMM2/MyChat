#include "RedisPool.hpp"  

RedisPool::RedisPool(const std::string& host, int port, int pool_size)  
    : host(host), port(port), maxPoolSize(pool_size) {  
    for (int i = 0; i < maxPoolSize; ++i) {  
        auto ctx = redisConnect(host.c_str(), port);  

        if (ctx == nullptr || ctx->err) {  
            if (ctx) {  
                std::cerr << "Error connecting to Redis: " << ctx->errstr << std::endl;  
                redisFree(ctx);  
            } else {  
                std::cerr << "Can't allocate Redis context" << std::endl;  
            }  
        } else {
        
            auto ctxPtr = std::unique_ptr<redisContext,std::function<void(redisContext*)>>(ctx , [this](redisContext* c){
                this->releaseConnection(c);
            });

            {
                std::lock_guard<std::mutex>lock(mtx);
                connectionPool.push(std::move(ctxPtr)); // 将连接加入池中  
            }
            
        }  
    }  
}  

RedisPool::~RedisPool() {  

}  

// 使用 unique_ptr 进行连接的获取  
std::unique_ptr<redisContext, std::function<void(redisContext*)>> RedisPool::getConnection() {  
    std::unique_lock<std::mutex> lock(mtx); // 加锁  
    cv.wait(lock, [this] { return !connectionPool.empty(); }); // 等待直到连接可用  

    auto connPtr = std::move(connectionPool.front());  
    connectionPool.pop();  
    
    //std::cout << "pool can use thread : " << connectionPool.size() << std::endl;

    return connPtr;
}  

void RedisPool::releaseConnection(redisContext* conn) {  
    if (conn) {  
        auto ctxPtr = std::unique_ptr<redisContext,std::function<void(redisContext*)>>(conn , [this](redisContext* c){
                this->releaseConnection(c);
            });
        {  
            std::lock_guard<std::mutex> lock(mtx);  
            connectionPool.push(std::move(ctxPtr)); // 将连接放回池中  
        }  
        //std::cout << "pool can use thread : " << connectionPool.size() << std::endl; 
        cv.notify_one(); // 通知等待的线程  
    }  
}