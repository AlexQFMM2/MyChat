#ifndef DATABASEMANAGER_H  
#define DATABASEMANAGER_H  

#include "DatabasePool.hpp"  
#include <string>  
#include <any>  
#include <vector>  
#include <memory>  
#include <iostream>  
#include <mysql_driver.h>  
#include <mysql_connection.h>  
#include <cppconn/prepared_statement.h> // 准备好的语句  
#include <cppconn/resultset.h>     // 结果集  
#include "MYJSON.hpp"  

using ROW = std::unordered_map<std::string, std::any>; // 使用映射来更好地表示行  
using TABLE = std::vector<ROW>; // 定义表格  

class DatabaseManager {  
public:  
    static DatabaseManager& getInstance(const json& config = json{}) {  
        static DatabaseManager instance(config); // 首次调用时创建实例  
        return instance;  
    }   

    // 执行 SQL 查询并返回结果  
    bool execSQL(const std::string& SQL, const std::vector<std::any>& params, TABLE& result);  

private:  
    DatabasePool pool; // 数据库连接池   

    // 私有构造函数，单例模式  
    DatabaseManager(const json& dbconfig);  

    // 私有克隆和赋值构造函数以防止复制  
    DatabaseManager(const DatabaseManager&) = delete;  
    DatabaseManager& operator=(const DatabaseManager&) = delete;  
};  

#endif

/*
    全局可访问性:

    单例模式确保一个类只有一个实例，并提供一个全局访问点。这对于需要在应用程序中共享的对象非常有用，例如数据库连接管理器。
    节省资源:

    创建和管理连接池或其他资源（如日志记录器、配置管理器等）时，用单例模式可以避免创建多个实例，从而节省内存和资源。
    集中管理:

    使用单例模式可以集中管理资源。例如，所有数据库连接都可以由一个 DatabaseManager 实例处理，避免资源浪费和状态不一致问题。
    惰性初始化:

    单例模式常常采用惰性初始化 ，即在首次需要时才创建实例。这可以避免在应用程序启动时加载不必要的资源。

    数据库管理: 需要在应用程序中维护数据库连接池并允许多个部分共享连接时。
    日志记录: 需要全局访问日志记录器，并且希望控制日志的输出以避免混合。
    配置管理: 应用程序的配置读取和管理都集中在一个管理类中。

    尽管单例模式有很多优点，但也要谨慎使用，因为：
    单例可能导致全局状态使得测试和调试变得困难，因为状态在不同测试之间共享。
    如果不合理设计，会对程序的灵活性和可扩展性造成影响。
    多线程环境下实施单例模式时需要特别小心，以免引起竞态条件问题。在 C++11 及更高版本中，使用局部静态变量可以简化线程安全问题。
*/

