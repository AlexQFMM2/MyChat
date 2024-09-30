#ifndef REDISMANAGER_H  
#define REDISMANAGER_H  

#include <string>  
#include <vector>  
#include <hiredis/hiredis.h>  
#include "RedisPool.hpp"  

class RedisManager {  
public:  
    RedisManager(const std::string& host, int port);  
    ~RedisManager();  

    void saveUserSession(const std::string& username, int client_sock);  
    void updateUserActivity(const std::string& username);  
    void checkUserTimeout();  
    int getUserSocket(const std::string& username);  
    std::vector<std::string> getAllActiveUsers();
    void print_online_user(std::vector<std::string>& activeUsers);
private:  
    RedisPool redis_pool; // Redis 连接池 
    long long wait_time; // 超时等待时间   
};  

#endif // REDISMANAGER_H