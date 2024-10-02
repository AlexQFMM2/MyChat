#ifndef RABBITMQMANAGER_H  
#define RABBITMQMANAGER_H  

#include "RabbitPool.hpp"  
#include "ChannelPool.hpp"  
#include "ConsumerPool.hpp"  
#include <string>  
#include <functional>  

class RabbitMqManager {  
public:  
    RabbitMqManager(const std::string& host, int port, const std::string& user, const std::string& password);  
    ~RabbitMqManager();  

    // 发布消息  
    void publishMessage(const std::string& exchange, const std::string& routingKey, const std::string& message);  

    // 开始消费消息  
    //“消费消息”指的是从队列中读取消息并进行处理的过程。具体来说，当一个消费者开始消费消息时，它会从指定队列中接收消息，然后根据需要执行某些操作，例如数据处理、数据库存储、触发其他服务等。
    void startConsuming(const std::string& queueName, std::function<void(const AMQP::Message&, uint64_t)> callback);  

private:  
    RabbitPool _rabbitPool;  
    ChannelPool _channelPool;  
    ConsumerPool _consumerPool;  
};  

#endif // RABBITMQMANAGER_H