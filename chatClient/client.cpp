#include "Client.hpp"

bool Client::Login(){
    connect_server();
    json json_data;
    json_data["who"] = cSock;
    json_data["command"] = "LOGIN";
    json_data["username"] = username;
    json_data["passwd"] = password;
    send_message(cSock, json_data);
    recv_server();
}

Client::~Client() {
    close(cSock);
}