#ifndef USERMANAGER_H
#define USERMANAGER_H

#include <vector>
#include <string>


class UserManager {
private:
    std::string username;
    int client_socket;
    long long last_activity_time;
public:
    UserManager(const std::string& Uname, int cSock , long long login_time);
    ~UserManager();

    // Getter 方法  
    std::string getUsername() const { return username; }  
    int getClientSocket() const { return client_socket; }  
    long long getLastActivityTime() const { return last_activity_time; }  

    // 更新活跃时间的方法  
    void updateLastActivityTime(long long time) {  
        last_activity_time = time;  
    }  
    
};


#endif