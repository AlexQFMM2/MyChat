#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "MYJSON.hpp"
#include "ThreadPool.hpp"

using json = nlohmann::json;

class Client {
private:
    int cSock;
    std::string username;
    std::string password;
    bool isConnected = false;
    std::thread listenerThread;

    std::mutex mtx;
    std::condition_variable cv;
    std::string confirmation_status;

    ThreadPool threadPool;  // 线程池对象


    void listen_to_server();
    void handle_chat_message(const std::string& friendname, const std::string& msg);


public:
    Client(int numThreads);
    bool connect_server();
    bool Login();
    bool Logout();
    bool send_chat_with_somebody(const std::string& friendName, const std::string& msg);
    void send_message(int sock, const json& message);
    void setUsername(const std::string& uname);  // 设置用户名
    void setPassword(const std::string& passwd);  // 设置密码
    ~Client();
};

#endif
