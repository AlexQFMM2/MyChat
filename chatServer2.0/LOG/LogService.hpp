#ifndef LOGSERVICE_H
#define LOGSERVICE_H

#include <string>
#include "KafkaLogger.hpp"

class LogService {
public:
    void logUserAction(const std::string& action, int userID);
    void logError(const std::string& errorMsg);
    void processLogs(); // 从 Kafka 中消费日志
private:
    KafkaLogger& kafkaLogger;
};

#endif