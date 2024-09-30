#include "DatabasePool.hpp"  
#include <stdexcept>  

DatabasePool::DatabasePool() = default;  

DatabasePool::~DatabasePool() {  
    // 释放连接池中的所有连接  
    while (!connectionPool.empty()) {  
        sql::Connection* conn = connectionPool.front();  
        delete conn;   
        connectionPool.pop();  
    }  
}  

void DatabasePool::initPool(json dbconfig) {  
    dbHost = dbconfig["host"];  
    dbUser = dbconfig["user"];  
    dbPassword = dbconfig["password"];  
    dbDatabase = dbconfig["DatabaseName"];  

    createPool(10); // 假设池的大小为10  
}  

void DatabasePool::createPool(size_t size) {  
    sql::Driver* driver = get_driver_instance();   
    for (size_t i = 0; i < size; ++i) {  
        sql::Connection* conn = driver->connect(dbHost, dbUser, dbPassword);  
        conn->setSchema(dbDatabase);  
        connectionPool.push(conn);  
    }  
}  

// 获取连接  
//std::unique_ptr<redisContext, std::function<void(redisContext*)>>
std::unique_ptr<sql::Connection , std::function<void(sql::Connection*)>> DatabasePool::getConnection() {  
    std::unique_lock<std::mutex> lock(poolMutex);  
    while (connectionPool.empty()) {  
        cv.wait(lock);   
    }  

    sql::Connection* conn = connectionPool.front();  
    connectionPool.pop();  

    //std::cout << "pool can use thread : " << connectionPool.size() << std::endl;

    // 使用 unique_ptr 管理连接的生命周期，使用默认删除器  
    return std::unique_ptr<sql::Connection , std::function<void(sql::Connection*)>>(conn , [this](sql::Connection* conn){
        this->releaseConnection(conn);
    });  
}  

// 释放连接  
void DatabasePool::releaseConnection(sql::Connection *conn) {  
    std::lock_guard<std::mutex> lock(poolMutex);  
    connectionPool.push(conn);  
    std::cout << "pool can use thread : " << connectionPool.size() << std::endl;
    cv.notify_one();   
}