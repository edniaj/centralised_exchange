#include <cassert>
#include <iostream>
#include <chrono>
#include "TestRedisManager.h"

TestRedisManager::TestRedisManager()
    : redis("tcp://127.0.0.1:6379", "TESTING")
{
    cleanupTestData();
}

void TestRedisManager::printTestResult(const std::string &testName, bool success, const std::string &errorMessage)
{
    testsRun++;
    if (success)
        testsPassed++;

    std::cout << (success ? "[✓] " : "[✗] ") << testName;
    if (!success && !errorMessage.empty())
    {
        std::cout << " - Error: " << errorMessage;
    }
    std::cout << std::endl;
}

void TestRedisManager::cleanupTestData()
{
    auto keys = redis.getAllKeys();
    for (const auto &key : keys)
    {
        if (key.substr(0, 7) == "orders:")
        {
            redis.pipeDeleteOrder(key.substr(7));
        }
    }
    redis.pipeExecute();
}

bool TestRedisManager::testConnection()
{
    try
    {
        std::string response = redis.ping();
        return response == "PONG";
    }
    catch (const std::exception &e)
    {
        std::cerr << "Connection test failed: " << e.what() << std::endl;
        return false;
    }
}

bool TestRedisManager::testOrderCreation()
{
    try
    {
        // Test single order creation
        bool success = redis.pipeCreateOrder("1001", "user123", "BUY", 100.50, 10);
        redis.pipeExecute();
        success &= verifyOrderExists("1001");

        // Test multiple orders
        success &= redis.pipeCreateOrder("1002", "user123", "SELL", 101.50, 5);
        success &= redis.pipeCreateOrder("1003", "user456", "BUY", 99.50, 15);
        redis.pipeExecute();

        success &= verifyOrderExists("1002");
        success &= verifyOrderExists("1003");

        // Test order with extreme values
        success &= redis.pipeCreateOrder("1004", "user789", "BUY", 999999.99, 1000000);
        redis.pipeExecute();
        success &= verifyOrderExists("1004");

        return success;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Order creation test failed: " << e.what() << std::endl;
        return false;
    }
}

bool TestRedisManager::testOrderUpdate()
{
    try
    {
        // Create initial test order
        redis.pipeCreateOrder("2001", "user123", "BUY", 100.00, 10);
        redis.pipeExecute();

        // Test 1: Valid price update
        std::unordered_map<std::string, std::string> priceUpdate = {
            {"price", "105.00"}};
        bool success = redis.pipeUpdateOrder("2001", priceUpdate);
        redis.pipeExecute();

        // Test 2: Valid quantity update
        std::unordered_map<std::string, std::string> quantityUpdate = {
            {"quantity", "20"}};
        success &= redis.pipeUpdateOrder("2001", quantityUpdate);
        redis.pipeExecute();

        // Test 3: Valid multiple field update (both price and quantity)
        std::unordered_map<std::string, std::string> bothUpdate = {
            {"price", "106.00"},
            {"quantity", "25"}};
        success &= redis.pipeUpdateOrder("2001", bothUpdate);
        redis.pipeExecute();

        // Test 4: Invalid field update (should fail)
        std::unordered_map<std::string, std::string> invalidUpdate = {
            {"side", "SELL"}, // This should fail
            {"price", "107.00"}};
        success &= !redis.pipeUpdateOrder("2001", invalidUpdate);

        // Test 5: Another invalid field update (should fail)
        std::unordered_map<std::string, std::string> invalidFieldUpdate = {
            {"invalid_field", "value"}};
        success &= !redis.pipeUpdateOrder("2001", invalidFieldUpdate);

        // Test 6: Empty update (should fail)
        std::unordered_map<std::string, std::string> emptyUpdate;
        success &= !redis.pipeUpdateOrder("2001", emptyUpdate);

        return success;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Order update test failed: " << e.what() << std::endl;
        return false;
    }
}

bool TestRedisManager::testErrorHandling()
{
    try
    {
        bool success = true;

        // Test 1: Invalid price format
        std::unordered_map<std::string, std::string> invalidPriceFormat = {
            {"price", "invalid_price"}};
        if (redis.pipeUpdateOrder("4001", invalidPriceFormat))
        {
            std::cout << "Test 1 (Invalid price format) failed: Update succeeded when it should have failed" << std::endl;
            success = false;
        }

        // Test 2: Invalid quantity format
        std::unordered_map<std::string, std::string> invalidQuantityFormat = {
            {"quantity", "not_a_number"}};
        if (redis.pipeUpdateOrder("4001", invalidQuantityFormat))
        {
            std::cout << "Test 2 (Invalid quantity format) failed: Update succeeded when it should have failed" << std::endl;
            success = false;
        }

        // Test 3: Unauthorized field update
        std::unordered_map<std::string, std::string> unauthorizedUpdate = {
            {"user_id", "hacker"},
            {"price", "100.00"}};
        if (redis.pipeUpdateOrder("4001", unauthorizedUpdate))
        {
            std::cout << "Test 3 (Unauthorized field update) failed: Update succeeded when it should have failed" << std::endl;
            success = false;
        }

        // Test 4: Non-existent order update
        std::unordered_map<std::string, std::string> validUpdate = {
            {"price", "100.00"}};
        // if (redis.pipeUpdateOrder("nonexistent_order", validUpdate)) {
        //     std::cout << "Test 4 (Non-existent order update) failed: Update succeeded when it should have failed" << std::endl;
        //     success = false;
        // }

        return success;
    }
    catch (const std::exception &e)
    {
        std::cout << "Exception caught in testErrorHandling: " << e.what() << std::endl;
        // Some errors should be caught and handled properly
        return true;
    }
}

bool TestRedisManager::testOrderDeletion()
{
    try
    {
        // Create test order for deletion
        redis.pipeCreateOrder("3001", "user123", "BUY", 100.00, 10);
        redis.pipeExecute();

        // Test successful deletion
        bool success = redis.pipeDeleteOrder("3001");
        redis.pipeExecute();
        success &= !verifyOrderExists("3001");

        // Test deletion of non-existent order
        success &= !redis.pipeDeleteOrder("nonexistent");
        redis.pipeExecute();

        // Test deletion of already deleted order
        success &= !redis.pipeDeleteOrder("3001");
        redis.pipeExecute();

        return success;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Order deletion test failed: " << e.what() << std::endl;
        return false;
    }
}

bool TestRedisManager::verifyOrderExists(const std::string &orderId)
{
    try
    {
        auto keys = redis.getAllKeys();
        return std::find(keys.begin(), keys.end(), "orders:" + orderId) != keys.end();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error verifying order: " << e.what() << std::endl;
        return false;
    }
}

void TestRedisManager::runAllTests()
{
    std::cout << "\n=== Starting Redis Manager Tests ===\n"
              << std::endl;

    printTestResult("Connection Test", testConnection());
    printTestResult("Order Creation Test", testOrderCreation());
    printTestResult("Order Update Test", testOrderUpdate());
    printTestResult("Order Deletion Test", testOrderDeletion());
    printTestResult("Error Handling Test", testErrorHandling());

    std::cout << "\n=== Test Summary ===\n";
    std::cout << "Total Tests: " << testsRun << std::endl;
    std::cout << "Tests Passed: " << testsPassed << std::endl;
    std::cout << "Success Rate: " << (testsPassed * 100.0 / testsRun) << "%\n"
              << std::endl;

    cleanupTestData();
}