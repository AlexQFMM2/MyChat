#include "DatabaseManager.hpp"

DatabaseManager::DatabaseManager(const json& dbconfig)
{
    pool.initPool(dbconfig); // 初始化连接池
    std::cout << "Database Pools is Waitting ...." << std::endl;
}

bool DatabaseManager::execSQL(const std::string& SQL, const std::vector<std::any>& params, TABLE& result) {  
    try {  
        // 获取连接  
        auto con = pool.getConnection();  

        std::unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement(SQL));  
        
        // 绑定参数  
        for (size_t i = 0; i < params.size(); ++i) {  
            if (params[i].type() == typeid(int)) {  
                pstmt->setInt(i + 1, std::any_cast<int>(params[i]));  
            } else if (params[i].type() == typeid(std::string)) {  
                pstmt->setString(i + 1, std::any_cast<std::string>(params[i]));  
            }  
            // 添加其他数据类型的处理  
        }  

        // 执行查询  
        std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());  
        
        while (res->next()) {  
            ROW row;  
            for (int i = 1; i <= res->getMetaData()->getColumnCount(); ++i) {  
                std::string columnName = res->getMetaData()->getColumnName(i);
                std::string value = res->getString(i);  // 获取当前列的值，假设所有列都为 VARCHAR 类型
                row[columnName] = value;  // 将列名和值存入结果
            }  
            result.push_back(row);  // 将这一行推入结果集
        }


    } catch (sql::SQLException& e) {  
        std::cerr << "SQLException: " << e.what() << std::endl;  
        std::cerr << "\tError code: " << e.getErrorCode() << std::endl;  
        std::cerr << "\tSQLState: " << e.getSQLState() << std::endl;  
        return false;  
    }  
    
    return true;
}