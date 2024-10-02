#ifndef CONSUMERPOOL_H  
#define CONSUMERPOOL_H  

#include "ChannelPool.hpp"  
#include <functional>  

class ConsumerPool {  
public:  
    ConsumerPool(ChannelPool& channelPool);  
    ~ConsumerPool();  

    void consumeMessage(const std::string& queueName, std::function<void(const AMQP::Message&, uint64_t)> callback);  

private:  
    ChannelPool& _channelPool; // 引用通道池  
};  

#endif // CONSUMERPOOL_H