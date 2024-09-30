#include "Client.hpp"
#include <iostream>
#include <unistd.h>  // For close()
#include <sys/socket.h>
#include <arpa/inet.h>

const std::string SEND_END = "|O@v.e^R|\n";

// 构造函数
Client::Client(int numThreads)
    : threadPool(numThreads)  // 初始化线程池
{
    const std::string ip = "192.168.10.100";
    const std::string port = "8888";

    // 创建Socket
    std::cout << "Connecting to IP: " << ip << " on port: " << port << std::endl;
    
    cSock = socket(AF_INET, SOCK_STREAM, 0);
    if (cSock < 0) {
        std::cerr << "Socket creation failed!" << std::endl;
        exit(1);
    }

    // 配置服务器地址
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(stoi(port));
    inet_pton(AF_INET, ip.c_str(), &serverAddr.sin_addr);

    // 连接服务器
    if (connect(cSock, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Connection to server failed!" << std::endl;
        exit(1);
    }

    // 启动监听任务
    threadPool.enqueue([this]() {
        listen_to_server();
    });
}

// 连接服务器
bool Client::connect_server() {
    if (!isConnected) {
        isConnected = true;
        return true;
    }
    return false;
}

// 发送消息给服务器
void Client::send_message(int sock, const json& message) {
    std::string data = message.dump() + SEND_END;
    send(sock, data.c_str(), data.size(), 0);
}

// 设置用户名
void Client::setUsername(const std::string& uname) {
    username = uname;
}

// 设置密码
void Client::setPassword(const std::string& passwd) {
    password = passwd;
}

// 析构函数
Client::~Client() {
    close_client();  // 关闭客户端

    std::cout << "Client is closed" << std::endl;
}

// 关闭客户端
void Client::close_client() {
    {
        std::lock_guard<std::mutex> lock(mtx);
        isConnected = false;  // 设置为不连接
        exitRequested = true;  // 设置退出请求
    }
    std::cout << "wait all thread exit...." << std::endl;
    cv.notify_all(); // 通知所有线程以检查退出请求

    // 关闭套接字
    if (cSock != -1) {  // 确保套接字有效
        close(cSock);
        std::cout << cSock << " is closed." << std::endl;
    }
}

void Client::listen_to_server() {
    int epollFd = epoll_create1(0);
    if (epollFd == -1) {
        std::cerr << "Failed to create epoll file descriptor" << std::endl;
        return;
    }

    epoll_event event;
    event.events = EPOLLIN;  // 监视可读事件
    event.data.fd = cSock;   // 添加到 epoll 中的 socket
    if (epoll_ctl(epollFd, EPOLL_CTL_ADD, cSock, &event) == -1) {
        std::cerr << "Failed to add socket to epoll" << std::endl;
        close(epollFd);
        return;
    }

    while (true) {
        epoll_event events[10]; // 事件数组
        int numEvents = epoll_wait(epollFd, events, 10, 1000); // 超时 1 秒

        // 检查是否请求退出
        {
            std::lock_guard<std::mutex> lock(mtx);
            if (exitRequested) {
                break;  // 退出线程
            }
        }  // 释放锁

        for (int i = 0; i < numEvents; ++i) {
            if (events[i].data.fd == cSock) {
                char buffer[1024];
                int len = recv(cSock, buffer, sizeof(buffer), 0);
                if (len <= 0) {
                    std::cerr << "Error or connection closed by server" << std::endl;
                    close(epollFd);  // 关闭 epoll
                    return; // 退出线程
                }

                std::string data(buffer, len);
                size_t pos = data.find(SEND_END);
                if (pos != std::string::npos) {
                    data.erase(pos, SEND_END.length());
                }

                try {
                    json request = json::parse(data);

                    // 根据不同类型将任务分配给线程池
                    if (request["command"] == "CHAT") {
                        threadPool.enqueue([this, request]() {
                            handle_chat_message(request["friendName"], request["message"]);
                        });
                    } else if (request["command"] == "CONFIRMATION") {
                        threadPool.enqueue([this, request]() {
                            std::lock_guard<std::mutex> lock(mtx);
                            confirmation_status = request["flag"] ? "SUCCESS" : "FAILURE";
                            std::cout << request["message"] << std::endl;
                            cv.notify_one();  // 唤醒等待的线程
                        });
                    } else {
                        std::cerr << "Unknown command received: " << request["command"] << std::endl;
                    }
                } catch (const std::exception& e) {
                    std::cerr << "JSON parse error: " << e.what() << std::endl;
                }
            }
        }
    }

    close(epollFd); // 清理 epoll 文件描述符
}

// 处理聊天消息
void Client::handle_chat_message(const std::string& friendname, const std::string& msg) {
    std::cout << "从 " << friendname << " 收到消息: " << msg << std::endl;
}

// 登录功能
bool Client::Login() {
    connect_server();
    json json_data;
    json_data["command"] = "LOGIN";
    json_data["username"] = username;
    json_data["passwd"] = password;
    
    send_message(cSock, json_data);

    // 使用条件变量等待确认消息
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [this] { return !confirmation_status.empty(); });

    bool login_success = (confirmation_status == "SUCCESS");
    confirmation_status.clear();  // 清空状态

    return login_success;
}

// 登出功能
bool Client::Logout() {
    json json_data;
    json_data["command"] = "LOGOUT";
    json_data["username"] = username;
    send_message(cSock, json_data);
    
    // 等待服务器确认登出
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [this] { return !confirmation_status.empty(); });
    
    bool logout_success = (confirmation_status == "SUCCESS");
    confirmation_status.clear();
    return logout_success;
}

// 发送聊天消息
// 发送聊天消息
bool Client::send_chat_with_somebody(const std::string& friendName, const std::string& msg) {
    json json_data;
    json_data["command"] = "CHAT";
    json_data["username"] = username;  // 发送者的用户名
    json_data["friendName"] = friendName;  // 目标好友的用户名
    json_data["message"] = msg;  // 发送的消息
    
    // 发送消息给服务器
    send_message(cSock, json_data);

    // 等待服务器的确认消息
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [this] { return !confirmation_status.empty(); });

    // 根据服务器的确认状态处理消息发送结果
    bool send_success = (confirmation_status == "SUCCESS");
    confirmation_status.clear();  // 清空状态
    return send_success;
}
