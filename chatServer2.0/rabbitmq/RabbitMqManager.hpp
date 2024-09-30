#ifndef RABBITMQMANAGER_H
#define RABBITMQMANAGER_H

#include <string>
#include "RabbitMqClient.hpp"

class RabbitMqManager {
public:
    static RabbitMqManager& getInstance();
    void sendMessage(int senderID, int recipientID, const std::string& message);
    std::string receiveMessage(int recipientID);
    // 其他 RabbitMQ 相关操作...
private:
    RabbitMqClient rabbitmq; // RabbitMQ 客户端
    RabbitMqManager();
};


#endif