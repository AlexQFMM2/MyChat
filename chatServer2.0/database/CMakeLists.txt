cmake_minimum_required(VERSION 3.10)  

project(DataBaseTest VERSION 1.0)  

# 设置 C++ 标准  
set(CMAKE_CXX_STANDARD 20)  
set(CMAKE_CXX_STANDARD_REQUIRED True)  

# 查找 MySQL 客户端库  
find_library(MYSQL_LIBRARIES NAMES mysqlclient PATHS /usr/lib/x86_64-linux-gnu)  

# 查找 MySQL Connector/C++ 库  
find_library(MYSQL_CONNECTOR_CPP_LIB mysqlcppconn PATHS /usr/lib/x86_64-linux-gnu)  

# 查找 MySQL 头文件  
find_path(MYSQL_INCLUDE_DIR NAMES mysql.h PATHS /usr/include/mysql)  
find_path(MYSQL_CONNECTOR_CPP_INCLUDE_DIR NAMES mysql_driver.h PATHS /usr/include/cppconn)  

# 检查所需的库和头文件是否找到  
if(NOT MYSQL_LIBRARIES)  
    message(FATAL_ERROR "MySQL library not found.")  
endif()  

if(NOT MYSQL_INCLUDE_DIR)  
    message(FATAL_ERROR "MySQL include directory not found.")  
endif()  

if(NOT MYSQL_CONNECTOR_CPP_LIB)  
    message(FATAL_ERROR "MySQL Connector/C++ library not found.")  
endif()  

if(NOT MYSQL_CONNECTOR_CPP_INCLUDE_DIR)  
    message(FATAL_ERROR "MySQL Connector/C++ include directory not found.")  
endif()  

# 添加可执行文件  
add_executable(DataBaseTest  
    DatabasePool.cpp  
    DatabaseManager.cpp  
    ../json/MYJSON.cpp  
    test.cpp  
)  

# 添加头文件搜索路径  
target_include_directories(DataBaseTest PRIVATE  
    "${CMAKE_CURRENT_SOURCE_DIR}"  
    "${MYSQL_INCLUDE_DIR}"  
    "${MYSQL_CONNECTOR_CPP_INCLUDE_DIR}"  # 添加 MySQL Connector/C++ 头文件路径  
    "${CMAKE_CURRENT_SOURCE_DIR}/../json"  # 添加 json 目录  
)  

# 链接 MySQL C API 库和 MySQL Connector/C++ 库  
target_link_libraries(DataBaseTest PRIVATE  
    ${MYSQL_LIBRARIES}  
    ${MYSQL_CONNECTOR_CPP_LIB}  # 添加 MySQL Connector/C++ 库链接  
)