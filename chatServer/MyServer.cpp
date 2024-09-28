#include "MyServer.hpp"
#include <mutex> // 为线程安全操作加入
#include <signal.h>
#include <map>
#include <set>
// 忽略 SIGPIPE 信号
std::mutex user_mutex;

std::mutex client_mutex; // 用于保护共享资源的互斥锁

void MyServer::error_handler(const std::string& msg) {
    std::cerr << msg << ": " << strerror(errno) << std::endl;
    exit(1); // 可以考虑将其换成抛出异常或日志记录，避免整个程序退出
}

void MyServer::set_nonblocking(int sock) {
    int flag = fcntl(sock, F_GETFL, 0);
    if (flag == -1) error_handler("fcntl get error");
    
    flag |= O_NONBLOCK; // 确保标志正确修改
    if (fcntl(sock, F_SETFL, flag) == -1) {
        error_handler("fcntl set non-blocking error");
    }
}

void MyServer::epoll_add(int type, int sock) {
    event.events = type | EPOLLET; // 使用边缘触发
    event.data.fd = sock;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, sock, &event) == -1) {
        error_handler("epoll_ctl() error");
    }
}

void MyServer::epoll_del(int sock) {
    if (epoll_ctl(epfd, EPOLL_CTL_DEL, sock, nullptr) == -1) {
        error_handler("epoll_ctl() delete error");
    }
    close(sock); // 关闭套接字
}

MyServer::MyServer(const std::string port) 
: sSock(socket(PF_INET, SOCK_STREAM, 0)), epfd(epoll_create1(0)), pool(10) {
    if (sSock < 0) {
        error_handler("socket error");
    }

    set_nonblocking(sSock);
    epoll_add(EPOLLIN, sSock);

    memset(&sAddr, 0, sizeof(sAddr));
    sAddr.sin_family = PF_INET;
    sAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    sAddr.sin_port = htons(stoi(port));

    signal(SIGPIPE, SIG_IGN);
    
    if (bind(sSock, (struct sockaddr*)&sAddr, sizeof(sAddr)) < 0) {
        error_handler("bind error");
    }

    if (listen(sSock, 20) < 0) {
        error_handler("listen error");
    }
}

MyServer::~MyServer() {
    epoll_del(sSock); // 删除 epoll 中的监听
    close(sSock);     // 关闭套接字
}

void MyServer::server_start() {
    ep_event.resize(50); // epoll 事件列表
    while (true) {
        event_cnt = epoll_wait(epfd, ep_event.data(), ep_event.size(), -1);
        if (event_cnt < 0) {
            error_handler("epoll_wait error");
        }

        std::cout << "epoll is listening..." << std::endl;
        std::cout << "Now listen the num of sock is " << event_cnt << std::endl;

        for (int i = 0; i < event_cnt; i++) {
            if (ep_event[i].data.fd == sSock) {
                accept_client(); // 接受新客户端
            } else {
                work(ep_event[i].data.fd); // 处理已有客户端的请求
            }
        }
    }
}

void MyServer::accept_client() {
    std::cout << "start accept() " << std::endl;
    socklen_t cAddr_sz = sizeof(cAddr); // 修改为 socklen_t 类型
    int cSock = accept(sSock, (struct sockaddr*)&cAddr, &cAddr_sz);

    if (cSock < 0) {
        error_handler("accept error");
        return;
    }

    set_nonblocking(cSock); // 设置非阻塞模式
    epoll_add(EPOLLIN, cSock); // 将新客户端加入 epoll
    std::cout << "Accepted new client: " << cSock << std::endl;
}

std::vector<json> MyServer::getcommand(int cSock) {
    static std::string total_msg;
    std::vector<json> commands;
    char buffer[1024];

    while (true) {
        ssize_t bytes_received = recv(cSock, buffer, sizeof(buffer) - 1, 0);

        if (bytes_received < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break; // 非阻塞下没有数据可读时正常退出
            } else if (errno == EINTR) {
                continue; // 被信号中断，重试
            } else {
                error_handler("recv error");
                return {}; // 返回空命令
            }

        } else if (bytes_received == 0) {
            std::cout << "Client disconnected." << std::endl;
            return {}; // 客户端断开连接
        }

        buffer[bytes_received] = '\0'; // 确保字符串结束
        total_msg += buffer; // 将接收到的消息添加到 total_msg

        size_t pos;
        while ((pos = total_msg.find(SEND_END)) != std::string::npos) {
            std::string command_str = total_msg.substr(0, pos);
            total_msg.erase(0, pos + SEND_END.length()); // 移除已处理的命令

            try {
                json command = json::parse(command_str); // 解析 JSON
                commands.push_back(command);
            } catch (const json::parse_error& e) {
                std::cerr << "JSON parse error: " << e.what() << std::endl;
            }
        }
    }

    return commands;
}

void MyServer::request_reponse(int cSock, json json_data)
{
    std::string msg = json_data.dump() + SEND_END;
    std::cout << msg << std::endl;
    send(cSock , msg.c_str() , msg.size() , 0);
}

void MyServer::work(int cSock) {
    std::cout << "start work() " << std::endl;

    std::vector<json> commands = getcommand(cSock);
    if (commands.empty()) {
        epoll_del(cSock); // 客户端断开连接，移除并关闭 socket
        std::cout << "Closed connection for socket: " << cSock << std::endl;
        return;
    }

    for (const json& command : commands) {
        pool.enqueue([this, command , cSock] {
            if (command.contains("command")) {
                std::string response = command["command"];
                //int who = command["who"];
                /*
                    为什么 who 不必要
                    command["who"] 是从客户端发送的数据中提取的，它本质上只是一个逻辑上的标识符，而不是系统分配的套接字。
                    使用 cSock 来确保每个任务对应的是正确的客户端连接比依赖客户端传来的 who 更加可靠。
                */
                if (response == "LOGIN") {
                    this->login(cSock, command["username"], command["passwd"]);
                } else if (response == "LOGOUT") {
                    this->logout(cSock , command["username"]);
                } else if (response == "CHAT") {
                    this->chat(cSock, command["friendName"], command["message"]);
                }
            }
        });
    }
}

//后续改为连接数据库
std::map<std::string,std::string>test_user = {{"alex","12345"},{"peter","12345"},{"sam" , "12345"}};
std::set<std::string>test_online = {};
std::map<std::string,int>online_hash_sock;
std::map<int,std::string>online_hash_username;

void print_online(){
    std::cout << "online : "; 
                for(auto s:test_online){
                    std::cout << s << " ";
                }
                std::cout << std::endl;
}

void MyServer::login(int cSock, const std::string& username, const std::string& password) {
    json json_data;
    json_data["command"] = "CONFIRMATION";
    json_data["flag"] = false;

    std::lock_guard<std::mutex> lock(user_mutex); // 加锁以确保线程安全

    if (test_user.count(username) > 0 && test_user[username] == password) {
        if (test_online.count(username) == 0) {
            // 用户在线状态更新
            test_online.insert(username);
            online_hash_sock[username] = cSock;
            online_hash_username[cSock] = username;

            json_data["flag"] = true;
            json_data["message"] = "登陆成功!";
            print_online(); // 打印在线用户
        } else {
            json_data["message"] = "你的账号已在线";
        }
    } else {
        json_data["message"] = "用户名或密码错误";
    }

    request_reponse(cSock, json_data);
}

//后面数据库就能不用username了，online表直接存username 和 cSock，用 cSock删除
void MyServer::logout(int cSock, const std::string& username) {
    json json_data;
    json_data["command"] = "CONFIRMATION";
    json_data["flag"] = false;

    std::lock_guard<std::mutex> lock(user_mutex); // 确保线程安全

    if (test_online.count(username) != 0) {
        test_online.erase(username);
        online_hash_username.erase(cSock);
        online_hash_sock.erase(username);

        json_data["flag"] = true;
        json_data["message"] = "登出成功!";
    } else {
        json_data["message"] = "登出失败，请稍后重试";
    }

    request_reponse(cSock, json_data);
}

void MyServer::chat(int cSock, const std::string &friendname, const std::string &msg) {
    json json_data;
    json_data["command"] = "CHAT";
    json_data["friendName"] = online_hash_username[cSock];
    json_data["message"] = msg;

    if (online_hash_sock.count(friendname) > 0) {
        int send_sock = online_hash_sock[friendname];
        std::string transpond = json_data.dump() + SEND_END; 
        send(send_sock, transpond.c_str(), transpond.size(), 0);
    } else {
        // 好友不在线，发送通知给发送者
        json_data.clear();  // 清除原来的消息内容
        json_data["command"] = "CONFIRMATION";  // 设定命令为错误通知
        json_data["flag"] = false;
        json_data["message"] = friendname + " 不在线.";  // 设定提示信息
        std::string error_response = json_data.dump() + SEND_END;
        send(cSock, error_response.c_str(), error_response.size(), 0);  // 发送错误消息
    }
}

