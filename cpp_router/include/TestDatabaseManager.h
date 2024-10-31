#pragma once

#include <string>
#include "DatabaseManager.h"

class TestDatabaseManager
{
private:
    DatabaseManager db;
    void printTestResult(const std::string &testName, bool success);

    // Helper functions for test setup and cleanup
    void cleanupUserTests();
    void cleanupOrderTests();
    void cleanupTradeTests();
    void cleanupBalanceTests();
    // Transaction Management Tests
    bool testStartPipe();
    bool testExecutePipe();
    bool testPipeActiveStatus();
    void runTransactionManagementTests();

    // User Table Tests
    bool testCreateUser();
    bool testVerifyUser();
    bool testReadUserByUsername();
    bool testReadSenderCompId();
    bool testReadAllUsers();
    bool testUpdateUser();
    bool testDeleteUser();
    void runUserTableTests();

    // Order Table Tests
    bool testPipeCreateOrder();
    bool testReadOrdersByUser();
    bool testReadOrdersBySymbol();
    bool testPipeUpdateOrderRemainingQuantity();
    bool testPipeUpdateOrderFilledQuantity();
    bool testPipeDeleteOrder();
    void runOrderTableTests();

    // Trade Table Tests
    bool testPipeCreateTrade();
    bool testReadTradesByUser();
    bool testReadTradesByOrder();
    void runTradeTableTests();

    // Balance Table Tests
    bool testCreateBalance();
    bool testReadBalancesByUser();
    bool testReadBalanceByUserAndAsset();
    bool testPipeUpdateBalance();
    void runBalanceTableTests();
    // Transaction Rollback Tests
    bool testTransactionRollback();

public:
    TestDatabaseManager(const std::string &connectionString);
    ~TestDatabaseManager();
    void runAllTests();
};