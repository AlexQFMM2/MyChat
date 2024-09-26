#include "MyServer.hpp"

MyServer::MyServer(const std::string port) 
: sSock(socket(PF_INET , SOCK_STREAM , 0)), epfd(epoll_create1(0)) , pool(10)
{
    if (sSock < 0) {
        error_handler("socket error");
    }

    set_nonblocking(sSock);
    epoll_add(EPOLLIN, sSock);

    memset(&sAddr, 0, sizeof(sAddr));
    sAddr.sin_family = PF_INET;
    sAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    sAddr.sin_port = htons(stoi(port));

    if (bind(sSock, (struct sockaddr *)&sAddr, sizeof(sAddr)) < 0) {
        error_handler("bind error");
    }

    if (listen(sSock, 20) < 0) {
        error_handler("listen error");
    }
}

MyServer::~MyServer()
{
    epoll_del(sSock);
    close(sSock);
}

void MyServer::server_start()
{
    ep_event.resize(50);
    while (true) {
        event_cnt = epoll_wait(epfd, ep_event.data(), ep_event.size(), -1);
        if (event_cnt < 0) {
            error_handler("epoll_wait error");
        }
        
        std::cout << "epoll is listening..." << std::endl;
        std::cout << "Now listen the num of sock is " << event_cnt << std::endl;
        
        for (int i = 0; i < event_cnt; i++) {
            if (ep_event[i].data.fd == sSock) {
                accept_client();
            } else {
                work(ep_event[i].data.fd);
                //std::cout << "work end() , epoll conntinue listen.." << std::endl;
            }
        }
    }
}

void MyServer::accept_client() {

    std::cout << "start accept() " << std::endl;
    cAddr_sz = sizeof(cAddr);
    cSock = accept(sSock, (struct sockaddr*)&cAddr, &cAddr_sz);
    if (cSock < 0) {
        error_handler("accept error");
        return; // 如果 accept 失败，直接返回
    }

    set_nonblocking(cSock); // 设置非阻塞
    epoll_add(EPOLLIN, cSock); // 将新连接加入到 epoll 监听
    std::cout << "Accepted new client: " << cSock << std::endl;
}


//非阻塞io，一次可能没读完，也有可能一次读到多条数据
//const std::string SEND_END = "|O@v.e^R|\n";
//用 SEND_END 判别一条命令的结束
std::vector<json> MyServer::getcommand(int cSock) {
    static std::string total_msg;
    std::vector<json> commands;
    char buffer[1024];

    while (true) {
        ssize_t bytes_received = recv(cSock, buffer, sizeof(buffer) - 1, 0);
        
        if (bytes_received < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // 没有更多数据可读，退出循环
                break;
            } else {
                error_handler("recv error");
                return {}; // 返回空命令
            }
        } else if (bytes_received == 0) {
            std::cout << "Client disconnected." << std::endl;
            return {}; // 客户端断开连接，返回空命令
        }

        buffer[bytes_received] = '\0'; // 确保字符串结束
        total_msg += buffer; // 将接收到的消息添加到 total_msg

        // 处理完整命令
        size_t pos;
        while ((pos = total_msg.find(SEND_END)) != std::string::npos) {
            std::string command_str = total_msg.substr(0, pos);
            total_msg.erase(0, pos + SEND_END.length()); // 移除已处理的命令

            // 将命令解析为 JSON
            try {
                json command = json::parse(command_str);
                commands.push_back(command);
            } catch (const json::parse_error& e) {
                std::cerr << "JSON parse error: " << e.what() << std::endl;
            }
        }
    }

    return commands;
}

void MyServer::work(int cSock) {

    std::cout << "start work() " << std::endl;
    
    while (true) {
        std::vector<json> commands = getcommand(cSock);

        if (commands.empty()) {
            // 客户端断开连接，移除并关闭 socket
            epoll_del(cSock);
            std::cout << "Closed connection for socket: " << cSock << std::endl;
            return;
        }

        for (const json& command : commands) {
            pool.enqueue([this, command] {
                if (command.contains("command") && command.contains("who")) {
                    std::string response = command["command"];
                    int who = command["who"];

                    if (response == "LOGIN") {
                        this->login(command["username"], command["passwd"]);
                    } else if (response == "LOGOUT") {
                        this->logout(who);
                    } else if (response == "CHAT") {
                        this->chat(who, command["friendname"], command["message"]);
                    }
                }
            });
        }
    }
}
