#include <iostream>  
#include "DatabaseManager.hpp"  
#include "MYJSON.hpp"  

using namespace std;  

// 函数声明  
void test_sql(const string& sql, const vector<any>& vars, DatabaseManager& db);  
void fit_dbconfig();  
void print_Table(const TABLE& res);  

json dbconfig;  

void fit_dbconfig() {  
    dbconfig["host"] = "localhost";  
    dbconfig["user"] = "root";  
    dbconfig["password"] = "257348619.ylY";  
    dbconfig["DatabaseName"] = "ChatServer";  
}  

void print_Table(const TABLE& res) {  
    std::cout << "--------------------------" << std::endl;  
    for (const ROW& row : res) {  
        for (const auto& it : row) {  
            std::string columnName = it.first; // 列名  
            const std::any& value = it.second; // 获取对应的值  

            if (value.type() == typeid(int)) {  
                std::cout << columnName << ": " << std::any_cast<int>(value) << " ";  
            } else if (value.type() == typeid(std::string)) {  
                std::cout << columnName << ": " << std::any_cast<std::string>(value)   
                          << " (size: " << std::any_cast<std::string>(value).size() << ") ";  
            } else {  
                std::cout << columnName << ": NULL "; // 处理 NULL 值  
            }  
        }  
        std::cout << std::endl; // 换行以区分不同的行  
    }  
    std::cout << "--------------------------" << std::endl;  
}  

int main() {  
    fit_dbconfig(); // 初始化数据库连接配置  

    // 获取并初始化单例  
    DatabaseManager& db = DatabaseManager::getInstance(dbconfig);  

    // 测试 SQL 查询  
    string sql = "SELECT * FROM Login where username = ? and password = ?"; // 或其他测试 SQL  
    std::vector<std::any> vars = {std::make_any<std::string>("user1"), std::make_any<std::string>("a12345")};

    //string sql = "delete from Login where username = ?";
    //std::vector<std::any> vars = {std::make_any<std::string>("user9")};
    cout << "exec sql...." << endl;
    test_sql(sql, vars, db); // 执行测试查询  

    return 0;  
}  

void test_sql(const string& sql, const vector<any>& vars, DatabaseManager& db) {  
    TABLE res; // 用于存储查询结果  

    if (db.execSQL(sql, vars, res)) {  
        print_Table(res); // 打印结果  
    } else {  
        cout << "SQL execution failed." << endl;  
    }  
}