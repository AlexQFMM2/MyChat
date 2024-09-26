#ifndef MYSERVER_H
#define MYSERVER_H

#include <iostream>
#include <arpa/inet.h>
#include <cstring>
#include <string>
#include <unistd.h>
#include <MYJSON.hpp>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <vector>
#include <fcntl.h>
#include "MYJSON.hpp"
#include "ThreadPool.hpp"

const std::string SEND_END = "|O@v.e^R|\n";

class MyServer{
    private:
        int sSock,cSock;
        struct sockaddr_in sAddr , cAddr;
        socklen_t cAddr_sz;
        struct epoll_event event;
        std::vector<epoll_event>ep_event;
        int epfd,event_cnt;
        ThreadPool pool;
        void error_handler(const std::string& msg){
            std::cerr << msg << ":" << strerror(errno)  << std::endl;
            exit(1);
        }

        void set_nonblocking(int sock) { //将套接字改为非阻塞模式
            int flag = fcntl(sock, F_GETFL, 0);
            if (flag == -1) error_handler("fcntl get error");
            if (fcntl(sock, F_SETFL, flag | O_NONBLOCK) == -1) {
                error_handler("fcntl set non-blocking error");
            }
        }

        void epoll_add(int type , int sock){
            event.events = type | EPOLLET; //监听读事件 OUT 就是写
            event.data.fd = sock; //将服务器套接字 sSock 的文件描述符赋值给 event 结构体中的 data 成员
            if (epoll_ctl(epfd, EPOLL_CTL_ADD, sock, &event) == -1) {
                error_handler("epoll_ctl() error");
            }
        }

        void epoll_del(int sock){
            if (epoll_ctl(epfd, EPOLL_CTL_DEL, sock, nullptr) == -1) {
                error_handler("epoll_ctl() delete error");
            }
            close(cSock);
        }

        void accept_client();

        void work(int cSock);

        std::vector<json> getcommand(int cSock);

        void login(const std::string& username , const std::string&password){
            std::cout << username << " & " << password << "\n is login !" << std::endl; 
        }

        void logout(int cSock){
            std::cout << cSock << " is logout " << std::endl; 
        }

        void chat(int cSock , const std::string& friendname , const std::string&msg){
            std::cout << cSock << " is send msg : " << msg << "\n to " << friendname << std::endl;
        }


    public:
        MyServer(const std::string port);
        ~MyServer();
        void server_start();
};


#endif