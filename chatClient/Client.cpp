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

    // 启动监听线程
    listenerThread = std::thread(&Client::listen_to_server, this);
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

// 接收服务器消息并加入线程池处理
void Client::listen_to_server() {
    while (true) {
        char buffer[1024];
        int len = recv(cSock, buffer, sizeof(buffer), 0);
        if (len <= 0) {
            std::cerr << "Error or connection closed by server" << std::endl;
            break;
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
                    cv.notify_one();
                });
            } else {
                std::cerr << "Unknown command received: " << request["command"] << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "JSON parse error: " << e.what() << std::endl;
        }
    }
}

// 处理聊天消息
void Client::handle_chat_message(const std::string& friendname, const std::string& msg) {
    std::cout << "从 " << friendname << " 收到消息: " << msg << std::endl;
}

// 析构函数
Client::~Client() {
    isConnected = false;
    close(cSock);
    if (listenerThread.joinable()) {
        listenerThread.join();  // 确保监听线程安全退出
    }
    std::cout << cSock << " is closed " << std::endl;
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
