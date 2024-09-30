#include "MyServer.hpp"

int main(int argc , char* argv[]){

    if(argc != 2){
        std::cerr << "Usage :"  << argv[0] << " <port> " << std::endl;
        exit(1);
    }

    MyServer server(argv[1],"ChatServer");
    server.server_start();

    return 0;
}