#pragma once

#include <string>
#include "DatabaseManager.h"

class TestDatabaseManager {
private:
    DatabaseManager db;
    
    void printTestResult(const std::string& testName, bool success);
    bool testCreateUser();
    bool testVerifyUser();
    bool testReadUserByUsername();
    bool testReadSenderCompId();
    bool testReadAllUsers();
    bool testUpdateUser();
    bool testDeleteUser();

public:
    TestDatabaseManager();
    void runAllTests();
};
