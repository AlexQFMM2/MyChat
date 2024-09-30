#include "RedisManager.hpp"  

std::mutex user_mutex; // 用于保护用户数据结构的互斥锁

RedisManager::RedisManager(const std::string& host, int port)  
    : redis_pool(host, port, 10), wait_time(25) { // 初始化连接池，设置超时等待时间  
}  

RedisManager::~RedisManager() {  
    // 连接池会在其析构函数中自动处理  
    std::cout << "RedisManager destroyed." << std::endl;  
}  

void RedisManager::saveUserSession(const std::string& username, int client_sock) {  
    auto conn = redis_pool.getConnection(); // 从连接池获取连接 

    // 获取当前时间并转换为秒  
    auto now_seconds = std::chrono::system_clock::now().time_since_epoch();  
    auto now = std::chrono::duration_cast<std::chrono::seconds>(now_seconds).count(); // 转换为秒  

    //std::cout << "[" << std::to_string(now).c_str() << "] : " << username << " is Login by socket : " << client_sock << std::endl;   
    
    // 保存用户 socket (以字符串形式存储)  
    redisCommand(conn.get(), "SET %s %d", username.c_str(), client_sock);  

    // 保存最后活动时间（以秒为字符串形式存储）  
    redisCommand(conn.get(), "SET %s:last_active %s", username.c_str(), std::to_string(now).c_str());   

    // 设置最后活动时间的过期时间  
    redisCommand(conn.get(), "EXPIRE %s:last_active %lld", username.c_str(), wait_time);  

     // 将用户添加到活跃用户集合中  
    redisCommand(conn.get(), "SADD active_users %s", username.c_str());  

    redis_pool.releaseConnection(conn.release());  // 释放 unique_ptr 并将连接放回
}

void RedisManager::updateUserActivity(const std::string& username) {  
    auto conn = redis_pool.getConnection(); // 从连接池获取连接    

    // 获取当前时间并转换为秒  
    auto now_seconds = std::chrono::system_clock::now().time_since_epoch();  
    auto now = std::chrono::duration_cast<std::chrono::seconds>(now_seconds).count(); // 转换为秒  

    // 更新用户最后活动时间（以秒为字符串形式存储）  
    redisCommand(conn.get(), "SET %s:last_active %s", username.c_str(), std::to_string(now).c_str());  

    // 可选：如果您希望在每次活动时更新过期时间，可以设置过期时间  
    redisCommand(conn.get(), "EXPIRE %s:last_active %lld", username.c_str(), wait_time); 

}

std::vector<std::string> RedisManager::getAllActiveUsers() {  
    auto conn = redis_pool.getConnection(); // 从连接池获取连接 

    std::vector<std::string> activeUsers; // 用于存储活跃用户列表  

    // 获取集合中的所有成员  
    redisReply* reply = (redisReply*)redisCommand(conn.get(), "SMEMBERS active_users");  
    
    if (reply && reply->type == REDIS_REPLY_ARRAY) {  
        for (size_t i = 0; i < reply->elements; ++i) {  
            activeUsers.emplace_back(reply->element[i]->str); // 将用户名添加到列表中  
        }  
    }  
    
    freeReplyObject(reply); // 释放 Redis 回复对象  

    return activeUsers; // 返回活跃用户列表  
}

void RedisManager::print_online_user(std::vector<std::string>& activeUsers)
{
    std::cout <<" now online user : " << std::endl;
    for(auto s:activeUsers){
        std::cout << s << " ";
    }
    std::cout << std::endl;
}

void RedisManager::checkUserTimeout() {  
    auto conn = redis_pool.getConnection(); // 从连接池获取连接  

    if(conn == nullptr){
        std::cout << "connect error" << std::endl;
        return;
    }

    // 获取当前时间并转换为秒  
    auto now_seconds = std::chrono::system_clock::now().time_since_epoch();  
    auto current_time = std::chrono::duration_cast<std::chrono::seconds>(now_seconds).count(); // 转换为秒  
    std::cout << "current_time is : " << current_time << std::endl;
    // 假设有一个获取所有用户会话的命令  
    std::vector<std::string> users = getAllActiveUsers(); // 需要实现此方法以获取所有活跃用户  

    //print_online_user(users);

    for (const auto& username : users) {  
        // 获取最后活动时间  
        std::cout << "---------------------------------" << std::endl;
        std::cout << username << ": " << std::endl;
        redisReply* reply = (redisReply*)redisCommand(conn.get(), "GET %s:last_active", username.c_str());  
        
        if (!reply) {  
            std::cerr << "Failed to execute command for user " << username << std::endl;  
            continue; // 跳过当前用户，继续下一个  
        }  

        if (reply->type == REDIS_REPLY_NIL) {  
            std::cout << "User " << username << " is no longer active (data not found)." << std::endl;  
            
            redisCommand(conn.get(), "DEL %s", username.c_str());  
            redisCommand(conn.get(), "DEL %s:sock", username.c_str());  
            redisCommand(conn.get(), "DEL %s:last_active", username.c_str());  
            
            freeReplyObject(reply); // 释放 reply 对象  
            continue; // 如果用户数据不在，跳过  
        }  

        if (reply && reply->type == REDIS_REPLY_STRING) {  
            long long last_active_time = std::stoll(reply->str); // 将字符串转换为 long long  
            std::cout << " last_active_time is : " << last_active_time << std::endl;
            std::cout << " no activity time is : " << current_time - last_active_time  << std::endl;
        }  

        freeReplyObject(reply); // 释放 Redis 回复对象  
        std::cout << "---------------------------------" << std::endl;
    }  
}

int RedisManager::getUserSocket(const std::string& username) {  
    auto conn = redis_pool.getConnection(); // 从连接池获取连接  

    redisReply* reply = (redisReply*)redisCommand(conn.get(), "GET %s", username.c_str());  

    int socket = -1;  
    if (reply) {  
        if (reply->type == REDIS_REPLY_STRING) {  
            socket = std::stoi(reply->str); // 将字符串转换为整数  
        } else {  
            std::cout << "No socket found for user: " << username << std::endl;  
        }  
        freeReplyObject(reply); // 释放 Redis 回复对象  
    } else {  
        std::cerr << "Failed to execute command: " << conn->errstr << std::endl;  
    }  
    return socket; // 返回 socket 值  
}