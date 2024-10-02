#ifndef REDISPOOL_H  
#define REDISPOOL_H  

#include <hiredis/hiredis.h>  //"/usr/include/hiredis"
#include <queue>  
#include <mutex>  
#include <string>  
#include <iostream>  
#include <stdexcept>  
#include <memory>  
#include <condition_variable>  
#include <functional>

class RedisPool {  
public:  
    RedisPool(const std::string& host, int port, int pool_size);  
    ~RedisPool();  

    void releaseConnection(redisContext* conn);  
    std::unique_ptr<redisContext, std::function<void(redisContext*)>> getConnection();  
    

private:  
    std::queue<std::unique_ptr<redisContext, std::function<void(redisContext*)>>> connectionPool; // 连接池  
    std::mutex mtx; // 互斥锁  
    int maxPoolSize; // 最大连接数  
    std::condition_variable cv; // 条件变量  

    std::string host;  
    int port;  
};  

#endif // REDISPOOL_H