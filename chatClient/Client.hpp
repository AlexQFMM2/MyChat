#ifndef CLIENT_H
#define CLIENT_H

#include "MYJSON.hpp"

#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

const std::string SEND_END = "|O@v.e^R|\n";
const std::string server_ip="192.168.10.100";
const std::string server_port = "8888";

class Client{
    private:
        int cSock;
        struct sockaddr_in sAddr;
        std::string username , password;
    public:
        Client() : cSock(socket(PF_INET , SOCK_STREAM , 0)){}

        ~Client();

        void setUserName(const std::string& username){
            this->username = username;
        }

        void setPassWord(const std::string& password){
            this->password = password;
        }

        void connect_server(){
            memset(&sAddr, 0, sizeof(sAddr));

            sAddr.sin_family = PF_INET;
            sAddr.sin_addr.s_addr =inet_addr(server_ip.c_str());
            sAddr.sin_port = htons(stoi(server_port));

            connect(cSock, (struct sockaddr*)&sAddr, sizeof(sAddr));
            std::cout << "The client sock is " << cSock << std::endl;
        }

        void send_message(int cSock , json json_data){
            std::string msg = json_data.dump() + SEND_END;
            send(cSock,msg.c_str(),msg.size() , 0);
        }

        void recv_server(){
            char buffer[1024];
            int str_len = recv(cSock , buffer , sizeof(buffer) - 1 , 0);
            
        }

        bool Login();

        void Logout() {
            json json_data;
            json_data["who"] = cSock;
            json_data["command"] = "LOGOUT";
            send_message(cSock , json_data);
        }     

        void send_chat_with_somebody(const std::string& friendName,const std::string& msg) {
            json json_data;
            json_data["who"] = cSock;
            json_data["command"] = "CHAT";
            json_data["friendname"] = friendName;
            json_data["message"] = msg;
 
            send_message(cSock , json_data);
        }
};

#endif