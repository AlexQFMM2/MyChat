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

        void error_handler(const std::string& msg);
        //将套接字改为非阻塞模式
        void set_nonblocking(int sock);

        void epoll_add(int type , int sock);

        void epoll_del(int sock);

        void accept_client();

        void work(int cSock);

        std::vector<json> getcommand(int cSock);

        void request_reponse(int cSock,json json_data);

    public:
        MyServer(const std::string port,const std::string dbname);
        ~MyServer();
        void server_start();
};


#endif