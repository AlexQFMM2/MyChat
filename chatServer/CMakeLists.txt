cmake_minimum_required(VERSION 3.10)

# 项目名称
project(ChatServer VERSION 1.0)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 查找需要的库
find_package(Threads REQUIRED)

# 查找 MySQL 库
find_library(MYSQL_LIBRARIES NAMES mysqlclient PATHS /usr/lib/x86_64-linux-gnu)
find_path(MYSQL_INCLUDE_DIR NAMES mysql.h PATHS /usr/include/mysql)

# 添加可执行文件
add_executable(ChatServer
    main.cpp
    MyServer.cpp
    MYJSON.cpp
    ThreadPool.cpp
    MyDatabase.cpp
)

# 包含头文件目录
target_include_directories(ChatServer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${MYSQL_INCLUDE_DIR} # 添加 MySQL 头文件目录
)

# 链接线程库和 MySQL 库
target_link_libraries(ChatServer PRIVATE
    Threads::Threads
    ${MYSQL_LIBRARIES} # 添加 MySQL 库
)
