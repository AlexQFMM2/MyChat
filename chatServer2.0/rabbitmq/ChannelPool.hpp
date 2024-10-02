#ifndef CHANNELPOOL_H  
#define CHANNELPOOL_H  

#include "amqpcpp.h"  
#include <queue>  
#include <mutex>  
#include <memory>  
#include <functional>  

class ChannelPool {  
public:  
    ChannelPool(std::shared_ptr<AMQP::Connection> connection);  
    ~ChannelPool();  

    //acquire : 获得
    std::unique_ptr<AMQP::Channel, std::function<void(AMQP::Channel*)>> acquireChannel();  
    void releaseChannel(std::unique_ptr<AMQP::Channel, std::function<void(AMQP::Channel*)>> channel);  

private:  
    std::queue<std::unique_ptr<AMQP::Channel, std::function<void(AMQP::Channel*)>>> _channels; // 通道池  
    std::mutex _mutex; // 确保线程安全  
    std::shared_ptr<AMQP::Connection> _connection; // 共享连接  

};  

#endif // CHANNELPOOL_H