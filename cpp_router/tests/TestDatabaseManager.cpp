#include <cassert>
#include <iostream>
#include "../include/TestDatabaseManager.h"

TestDatabaseManager::TestDatabaseManager() 
    : db("dbname=docker host=localhost user=docker password=docker")
{}

void TestDatabaseManager::printTestResult(const std::string& testName, bool success) {
    std::cout << (success ? "[✓] " : "[✗] ") << testName << std::endl;
}

bool TestDatabaseManager::testCreateUser() {
    bool success = db.createUser("testuser", "testpass", "TEST123");
    printTestResult("Create User", success);
    return success;
}

bool TestDatabaseManager::testVerifyUser() {
    bool success = db.verifyUser("testuser", "testpass");
    printTestResult("Verify User", success);
    return success;
}

bool TestDatabaseManager::testReadUserByUsername() {
    auto user = db.readUserByUsername("testuser");
    bool success = user.has_value() && user->username == "testuser";
    printTestResult("Read User By Username", success);
    return success;
}

bool TestDatabaseManager::testReadSenderCompId() {
    auto senderCompId = db.readSenderCompIdByUsername("testuser");
    bool success = senderCompId.has_value() && senderCompId.value() == "TEST123";
    printTestResult("Read SenderCompId", success);
    return success;
}

bool TestDatabaseManager::testReadAllUsers() {
    auto users = db.readAllUsers();
    bool success = !users.empty();
    printTestResult("Read All Users", success);
    return success;
}

bool TestDatabaseManager::testUpdateUser() {
    auto user = db.readUserByUsername("testuser");
    if (!user.has_value()) return false;
    
    bool success = db.updateUser(user->id, "newpass", "TEST456");
    printTestResult("Update User", success);
    return success;
}

bool TestDatabaseManager::testDeleteUser() {
    auto user = db.readUserByUsername("testuser");
    if (!user.has_value()) return false;
    
    bool success = db.deleteUser(user->id);
    printTestResult("Delete User", success);
    return success;
}

void TestDatabaseManager::runAllTests() {
    std::cout << "\n=== Starting Database Tests ===\n" << std::endl;
    
    // Run tests in sequence
    bool createSuccess = testCreateUser();
    if (createSuccess) {
        testVerifyUser();
        testReadUserByUsername();
        testReadSenderCompId();
        testReadAllUsers();
        testUpdateUser();
        testDeleteUser();
    } else {
        std::cout << "Create user failed, skipping remaining tests." << std::endl;
    }

    std::cout << "\n=== Database Tests Complete ===\n" << std::endl;
}
