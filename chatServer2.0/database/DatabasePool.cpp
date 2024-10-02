#include "DatabasePool.hpp"  
#include <stdexcept>  

DatabasePool::DatabasePool() = default;  

DatabasePool::~DatabasePool() {  
   
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
        auto conn = driver->connect(dbHost, dbUser, dbPassword);  
        conn->setSchema(dbDatabase);  

        auto connPtr = std::unique_ptr<sql::Connection, std::function<void(sql::Connection*)>>(conn, [this](sql::Connection*conn) {  
            this->releaseConnection(conn);  
        });  

        {  
            std::lock_guard<std::mutex> lock(poolMutex);  
            connectionPool.push(std::move(connPtr));  
        }  
    }  
}

// 获取连接  
//std::unique_ptr<redisContext, std::function<void(redisContext*)>>
std::unique_ptr<sql::Connection, std::function<void(sql::Connection*)>> DatabasePool::getConnection() {  
    std::unique_lock<std::mutex> lock(poolMutex);  
    while (connectionPool.empty()) {  
        cv.wait(lock);  
    }  
    
    auto connPtr = std::move(connectionPool.front());  
    connectionPool.pop();  

    //std::cout << "now connect has: " << connectionPool.size() << std::endl;

    return connPtr;  
}

void DatabasePool::releaseConnection(sql::Connection *conn) {  
    bool isConnectionValid = true;  

    try {  
        // 检查连接是否有效，执行一个简单的查询  
        std::unique_ptr<sql::Statement> stmt(conn->createStatement());  
        stmt->execute("SELECT * FROM test");  
    } catch (const sql::SQLException& e) {  
        isConnectionValid = false;  
    }  

    if (!isConnectionValid) {  
        try {  
            // 重新初始化连接  
            sql::Driver* driver = get_driver_instance();  
            sql::Connection* newConn = driver->connect(dbHost, dbUser, dbPassword);  
            newConn->setSchema(dbDatabase);  
            conn = newConn;  
        } catch (const sql::SQLException& e) {  
            // 记录失败信息并防止添加连接到池中  
            // 这里你可以选择记录日志以了解初始化失败的原因  
            delete conn;   
            return;  
        }  
    }  
    
    // 将连接入池  
    {  
        std::unique_lock<std::mutex> lock(poolMutex);  

        // 清空原有连接，替换为新连接，并放回队列  
        connectionPool.push(std::unique_ptr<sql::Connection, std::function<void(sql::Connection*)>>(  
            conn, [this](sql::Connection*conn) {  
                this->releaseConnection(conn);  
            }));  

        // 通知等待的线程有可用连接  
        cv.notify_one();  
    }  
}