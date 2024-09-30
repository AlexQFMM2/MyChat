#ifndef KAFKALOGGER_H
#define KAFKALOGGER_H

#include <string>
#include "KafkaProducer.hpp"

class KafkaLogger {
public:
    static KafkaLogger& getInstance();
    void logUserLogin(int userID, const std::string& ipAddress);
    void logUserLogout(int userID);
    void logChatMessage(int senderID, int recipientID, const std::string& message);
    void logSystemError(const std::string& errorMsg);
private:
    KafkaProducer producer; // Kafka 生产者
    KafkaLogger();
};

#endif
