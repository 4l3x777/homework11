#include "sqlite3.h"
#include <string>
#include <iostream>

namespace sqlite3cpp 
{
static std::string result;
static int calculate_result(void*, int columns, char **data, char **names)
{
    for (int i = 0; i < columns; ++i) result += (data[i] ? data[i] : "") + std::string("\n");
    return 0;
}

class SQLDB
{
    sqlite3* handle {nullptr};

public:

    bool initDB()
    {
        const char* db_name = ":memory:";
        if (sqlite3_open(db_name, &handle))
        {
            sqlite3_close(handle);
            return false;
        }

        // create 2 tables (A & B)
        if(
            request("CREATE TABLE A (id INT PRIMARY KEY,name VARCHAR(255))") != "OK" ||
            request("CREATE TABLE B (id INT PRIMARY KEY,name VARCHAR(255))") != "OK"
        )
        {
            return false;
        }
        else return true;
    }

    std::string request(std::string req)
    {   
        result.clear();
        char *errmsg;
        int rc = sqlite3_exec(handle, req.data(), calculate_result, 0, &errmsg);
        if (rc != SQLITE_OK)
        {
            auto error = std::string(errmsg);
            sqlite3_free(errmsg);
            return error;
        }
        else
        {
            return result.empty() ? "OK" : result + "OK";
        }
    }

    ~SQLDB()
    {
        sqlite3_close(handle);
    }

};
}