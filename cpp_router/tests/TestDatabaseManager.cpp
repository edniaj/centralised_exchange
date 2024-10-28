#include <cassert>
#include <iostream>
#include "../include/DatabaseManager.h"

class TestDatabaseManager {
private:
    DatabaseManager& db;
    
    void printTestResult(const std::string& testName, bool success) {
        std::cout << (success ? "[✓] " : "[✗] ") << testName << std::endl;
    }

    bool testCreateUser() {
        bool success = db.createUser("testuser", "testpass", "TEST123");
        printTestResult("Create User", success);
        return success;
    }

    bool testVerifyUser() {
        bool success = db.verifyUser("testuser", "testpass");
        printTestResult("Verify User", success);
        return success;
    }

    bool testReadUserByUsername() {
        auto user = db.readUserByUsername("testuser");
        bool success = user.has_value() && user->username == "testuser";
        printTestResult("Read User By Username", success);
        return success;
    }

    bool testReadSenderCompId() {
        auto senderCompId = db.readSenderCompIdByUsername("testuser");
        bool success = senderCompId.has_value() && senderCompId.value() == "TEST123";
        printTestResult("Read SenderCompId", success);
        return success;
    }

    bool testReadAllUsers() {
        auto users = db.readAllUsers();
        bool success = !users.empty();
        printTestResult("Read All Users", success);
        return success;
    }

    bool testUpdateUser() {
        auto user = db.readUserByUsername("testuser");
        if (!user.has_value()) return false;
        
        bool success = db.updateUser(user->id, "newpass", "TEST456");
        printTestResult("Update User", success);
        return success;
    }

    bool testDeleteUser() {
        auto user = db.readUserByUsername("testuser");
        if (!user.has_value()) return false;
        
        bool success = db.deleteUser(user->id);
        printTestResult("Delete User", success);
        return success;
    }

public:
    TestDatabaseManager(DatabaseManager& dbManager) : db(dbManager) {}

    void runAllTests() {
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
};

// Example usage:
int main() {
    std::cout << "Starting database tests..." << std::endl;
    try {
        std::string connString = "dbname=docker host=localhost user=docker password=docker";
        DatabaseManager dbManager(connString);
        TestDatabaseManager tester(dbManager);
        tester.runAllTests();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with error: " << e.what() << std::endl;
        return 1;
    }
}
