#include "Client.hpp"
#include <iostream>


std::string uName, pWord;
Client user(4);  

void startMenuGUI() {
    puts("-----------------------");
    puts("Login Menu");
    puts("1. Login");
    puts("2. EXIT");
    puts("-----------------------");
}

void userMenuGUI() {
    puts("-----------------------");
    puts("User Menu");
    puts("1. CHAT");
    puts("2. LOGOUT");
    puts("-----------------------");
}

void userMenu() {
    while (1) {
        userMenuGUI();
        std::string op;
        getline(std::cin, op);
        if (op[0] == '1') {
            std::string friendName, msg;
            std::cout << "输入要聊天的用户名: ";
            getline(std::cin, friendName);
            std::cout << "输入消息: ";
            getline(std::cin, msg);
            user.send_chat_with_somebody(friendName, msg);
        } else if (op[0] == '2') {
            if (user.Logout())
                break;
        }
    }
}

void startMenu() {
    startMenuGUI();
    std::string op;
    getline(std::cin, op);
    if (op[0] == '1') {
        puts("Insert Your UserName: ");
        getline(std::cin, uName);
        puts("Insert Your PassWord: ");
        getline(std::cin, pWord);
        user.setUsername(uName);  // 使用 setUsername
        user.setPassword(pWord);  // 使用 setPassword

        if (user.Login()) {
            userMenu();
        }
    } else if (op[0] == '2') {
        std::cout << "close client..." << std::endl;
        user.close_client();
        exit(0);
    }    
}

int main() {
    std::cout <<  "Client is start!" << std::endl;
    while (1) {
        startMenu();
    }
    return 0;
}
