#include <iostream>  
#include <thread>  
#include <chrono>  
#include "RedisManager.hpp"  
#include <string>

using namespace std;

void print_sock(RedisManager& redisManager);  

int main() {  
    // 创建 RedisManager 实例，连接到 Redis 服务器  
    RedisManager redisManager("127.0.0.1", 6379);  

    // 保存用户会话  
    std::cout << "Saving user sessions..." << std::endl;  
    redisManager.saveUserSession("user1", 1001);  
    redisManager.saveUserSession("user2", 1002);  
    redisManager.saveUserSession("user3", 1003);  

    vector<string>online = redisManager.getAllActiveUsers();
    redisManager.print_online_user(online);

    print_sock(redisManager);  

    for(int i = 0 ; i < 3 ; i ++){  
        // 更新用户活动  
        std::cout << "Updating user activities..." << std::endl;  
        redisManager.updateUserActivity("user1");  
        if(i == 1) redisManager.updateUserActivity("user2");  

        // 等待 10 秒以测试超时  
        std::cout << "Waiting for 10 seconds to check user timeout..." << std::endl;  
        std::this_thread::sleep_for(std::chrono::seconds(10));  

        // 检查用户超时  
        std::cout << "Checking user timeouts..." << std::endl;  
        redisManager.checkUserTimeout();  
        
        print_sock(redisManager);  
    }  

    return 0;  
}  

void print_sock(RedisManager& redisManager) {  

        std::cout << "---------------------------------" << std::endl;  
        
        //std::cout << "Find User1 socket: " << std::endl;
        int socket1 =redisManager.getUserSocket("user1");
        std::cout << "User1 socket: " << socket1 << std::endl;   

        //std::cout << "Find User2 socket: " << std::endl;
        int socket2 = redisManager.getUserSocket("user2");
        std::cout << "User2 socket: " << socket2 << std::endl;   
        
        //std::cout << "Find User3 socket: " << std::endl;
        int socket3 = redisManager.getUserSocket("user3");
        std::cout << "User3 socket: " << socket3 << std::endl;  

        std::cout << "---------------------------------" << std::endl; 
}