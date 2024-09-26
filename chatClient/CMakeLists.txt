cmake_minimum_required(VERSION 3.10)

project(ChatClient VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(ChatClient
    main.cpp
    MYJSON.cpp
    Client.cpp
)

target_include_directories(ChatClient PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
)

