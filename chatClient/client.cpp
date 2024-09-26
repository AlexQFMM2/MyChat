#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "json.hpp"

using namespace std;
using json = nlohmann::json;


const std::string SEND_END = "|O@v.e^R|\n";
const std::string server_ip="192.168.10.100";
const std::string server_port = "8888";

class Client{
    private:
        int cSock;
        struct sockaddr_in sAddr;
        string username , password;
    public:
        Client() : cSock(socket(PF_INET , SOCK_STREAM , 0)){}

        ~Client();

        void setUserName(const string& username){
            this->username = username;
        }

        void setPassWord(const string& password){
            this->password = password;
        }

        void connect_server(){
            memset(&sAddr, 0, sizeof(sAddr));

            sAddr.sin_family = PF_INET;
            sAddr.sin_addr.s_addr =inet_addr(server_ip.c_str());
            sAddr.sin_port = htons(stoi(server_port));

            connect(cSock, (struct sockaddr*)&sAddr, sizeof(sAddr));
            cout << "The client sock is " << cSock << endl;
        }

        void send_message(int cSock , json json_data){
            string msg = json_data.dump() + SEND_END;
            send(cSock,msg.c_str(),msg.size() , 0);
        }

        void Login();
        
};


int main(int argc, char* argv[]) {

    Client user;

    user.setUserName("alexqfmm");
    user.setPassWord("123456");

    user.Login();

    return 0;
}
