#ifndef CHATSERVICE_H
#define CHATSERVICE_H

#include <string>
#include "RabbitMqManager.hpp"
#include "RedisManager.hpp"
#include "KafkaLogger.hpp"

class ChatService {
public:
    void sendMessage(int senderID, int recipientID, const std::string& message);
    void receiveMessage(int recipientID);
private:
    RabbitMqManager& rabbitMQManager;
    RedisManager& redisManager;
    KafkaLogger& kafkaLogger;
};


#endif