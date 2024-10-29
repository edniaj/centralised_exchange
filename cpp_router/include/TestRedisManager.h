#pragma once

#include <string>
#include <unordered_map>
#include "RedisManager.h"

class TestRedisManager {
private:
    RedisManager redis;
    int testsRun = 0;
    int testsPassed = 0;

    // Helper methods
    void printTestResult(const std::string& testName, bool success, const std::string& errorMessage = "");
    void cleanupTestData();
    bool verifyOrderExists(const std::string& orderId);

    // Individual test methods
    bool testConnection();
    bool testOrderCreation();
    bool testOrderUpdate();
    bool testOrderDeletion();
    bool testErrorHandling();

public:
    // Constructor
    TestRedisManager();
    
    // Main test runner
    void runAllTests();
}; 