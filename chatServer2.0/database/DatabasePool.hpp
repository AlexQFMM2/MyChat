#ifndef DATABASEPOOL_H  
#define DATABASEPOOL_H  

#include <memory>  
#include <string>  
#include <queue>  
#include <mutex>  
#include <condition_variable>  
#include <cppconn/driver.h>  
#include <cppconn/connection.h>  
#include <cppconn/statement.h>  
#include <functional>
#include "MYJSON.hpp"  

class DatabasePool  
{  
public:  
    DatabasePool();  
    ~DatabasePool();  
    void initPool(json dbconfig); // 初始化连接池  
    std::unique_ptr<sql::Connection , std::function<void(sql::Connection*)>> getConnection(); // 获取连接  
    void releaseConnection(sql::Connection *conn); // 释放连接  

private:  
    void createPool(size_t size); // 创建连接池  

    std::queue<std::unique_ptr<sql::Connection , std::function<void(sql::Connection*)>>> connectionPool; // 连接池  
    std::mutex poolMutex; // 互斥量  
    std::condition_variable cv; // 条件变量  

    std::string dbHost;     // 数据库主机  
    std::string dbUser;     // 数据库用户  
    std::string dbPassword; // 数据库密码  
    std::string dbDatabase; // 数据库名称  
};  

#endif // DATABASEPOOL_H