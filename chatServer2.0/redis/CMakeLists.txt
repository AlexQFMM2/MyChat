cmake_minimum_required(VERSION 3.10)  

# 项目名称和版本  
project(RedisManagerTest VERSION 1.0)  

# 查找 hiredis 库  
find_library(HIREDIS_LIB hiredis REQUIRED)  

# 包含头文件目录  
include_directories(${CMAKE_SOURCE_DIR})  

# 添加 RedisManager 和 RedisPool 源文件  
set(SOURCE_FILES  
    RedisManager.cpp  
    RedisPool.cpp  
)  

# 创建 RedisManager 库  
add_library(RedisManager ${SOURCE_FILES})  

# 创建可执行测试程序  
add_executable(test test.cpp)  

# 链接 RedisManager 库和 hiredis 库到测试程序  
target_link_libraries(test RedisManager ${HIREDIS_LIB} pthread)  

# 可选：设置 C++ 标准  
set_target_properties(RedisManager PROPERTIES  
    CXX_STANDARD 11  
    CXX_STANDARD_REQUIRED ON  
)  

set_target_properties(test PROPERTIES  
    CXX_STANDARD 11  
    CXX_STANDARD_REQUIRED ON  
)  