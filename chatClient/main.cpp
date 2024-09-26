#include "Client.hpp"

std::string op , uName,pWord,msg;

void startMenuGUI()
{
    puts("-----------------------");
    puts("Login Menu");
    puts("1.Login");
    puts("2.EXIT");
    puts("-----------------------");
}

int main()
{
    Client user;
    
    startMenuGUI();

    while(1){
        getline(std::cin , op);
        if(op[0] == '1'){
            puts("Insert Your UserName : ");
            getline(std::cin,uName);
            puts("Insert Your PassWord : ");
            getline(std::cin,pWord);
            user.setUserName(uName);
            user.setPassWord(pWord);
            user.Login();
        }else if(op[0] == '2'){
            exit(0);
        }
    }

    return 0;
}