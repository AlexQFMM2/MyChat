#ifndef RABBITPOOL_H  
#define RABBITPOOL_H  

#include "amqpcpp.h"  
#include "amqpcpp/linux_tcp.h"  
#include <queue>  
#include <mutex>  
#include <memory>  
#include <string>  
#include <functional>  

class RabbitPool {  
public:  
    RabbitPool(const std::string& host, int port, const std::string& user, const std::string& password);  
    ~RabbitPool();  

    std::unique_ptr<AMQP::Connection, std::function<void(AMQP::Connection*)>> acquireConnection();  
    void releaseConnection(std::unique_ptr<AMQP::Connection, std::function<void(AMQP::Connection*)>> connection);  

private:  
    std::queue<std::unique_ptr<AMQP::Connection, std::function<void(AMQP::Connection*)>>> _connections; // 连接池  
    std::mutex _mutex; // 确保线程安全  
    std::string _host;  
    int _port;  
    std::string _user;  
    std::string _password;  

};  

#endif // RABBITPOOL_H