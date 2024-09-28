cmake_minimum_required(VERSION 3.10)

project(ChatClient VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(Threads REQUIRED)

add_executable(ChatClient
    main.cpp
    MYJSON.cpp
    Client.cpp
    ThreadPool.cpp
)

target_include_directories(ChatClient PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# 链接线程库
target_link_libraries(ChatClient PRIVATE
    Threads::Threads
)

