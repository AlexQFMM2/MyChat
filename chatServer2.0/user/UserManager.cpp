#include "UserManager.hpp"

UserManager::UserManager(const std::string &Uname, int cSock, long long login_time)
:username(Uname) , client_socket(cSock) , last_activity_time(login_time) {}

UserManager::~UserManager() {}

