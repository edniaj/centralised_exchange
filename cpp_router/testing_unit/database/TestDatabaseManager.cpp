#include <cassert>
#include <iostream>
#include "../include/TestDatabaseManager.h"

/*
Testing Style Guide:
1. Test Runner Method (e.g., runTradeTableTests):
   - Should only orchestrate the test execution
   - Performs initial cleanup of all dependent tables
   - Calls individual test methods
   - Prints results
   - Performs final cleanup
   - Does NOT contain test setup logic

2. Individual Test Methods:
   - Must be fully self-contained
   - Responsible for their own setup (creating users, orders, etc.)
   - Must clean up after themselves
   - Should handle their own error cases
   - Should use try-catch blocks for error handling
   - Return boolean indicating success/failure

3. Cleanup Order:
   - Always clean from most dependent to least dependent
   - Example: trades -> orders -> users
   - Both before and after test execution

4. Error Handling:
   - Early return if critical setup fails
   - Proper cleanup even in failure cases
   - Descriptive error messages in catch blocks
*/

// Write test case style - 1. Whenever there is an update or delete, make sure we have a created the data.
// We have a function to clean up all the data. Wipe it clean. The unit test should not affect the other tests.
// UNIT TEST like order or trades or balance, we have to create the data. literally frmo scratch. assume no data for each unit testing
TestDatabaseManager::TestDatabaseManager(const std::string &connectionString)
    : db(connectionString)
{
    cleanupUserTests();
    cleanupOrderTests();
    cleanupTradeTests();
    cleanupBalanceTests();
}

TestDatabaseManager::~TestDatabaseManager()
{
    try
    {
        // First check and handle any active transaction
        if (db.isPipeActive())
        {
            try
            {
                db.executePipe(); // Try to commit any pending transaction
            }
            catch (...)
            {
                // If commit fails, force a new transaction for cleanup
                db.startPipe();
            }
        }

        // Now perform cleanup with fresh transactions
        pqxx::work txn(db.getConnection());

        // Delete from all tables in correct order (respecting foreign key constraints)
        txn.exec("DELETE FROM trades;");
        txn.exec("DELETE FROM orders;");
        txn.exec("DELETE FROM balances;");
        txn.exec("DELETE FROM users;");

        txn.commit();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error during final cleanup: " << e.what() << std::endl;
    }
    catch (...)
    {
        std::cerr << "Unknown error during final cleanup" << std::endl;
    }
}

void TestDatabaseManager::printTestResult(const std::string &testName, bool success)
{
    std::cout << (success ? "[✓] " : "[✗] ") << testName << std::endl;
}

// Transaction Management Tests

bool TestDatabaseManager::testStartPipe()
{
    return db.startPipe();
}

bool TestDatabaseManager::testExecutePipe()
{
    return db.executePipe();
}

bool TestDatabaseManager::testPipeActiveStatus()
{
    bool initialStatus = db.isPipeActive();
    db.startPipe();
    bool activeStatus = db.isPipeActive();
    db.executePipe();
    bool finalStatus = db.isPipeActive();

    return !initialStatus && activeStatus && !finalStatus;
}

void TestDatabaseManager::runTransactionManagementTests()
{
    std::cout << "\n=== Running Transaction Management Tests ===\n";

    bool startPipeSuccess = testStartPipe();
    printTestResult("Start Pipe Test", startPipeSuccess);

    bool executePipeSuccess = testExecutePipe();
    printTestResult("Execute Pipe Test", executePipeSuccess);

    bool pipeActiveStatusSuccess = testPipeActiveStatus();
    printTestResult("Pipe Active Status Test", pipeActiveStatusSuccess);

    std::cout << "=== Transaction Management Tests Complete ===\n";
}

// User Table Tests

bool TestDatabaseManager::testCreateUser()
{
    // Perform the test
    bool success = db.createUser("testuser", "testpass", 123);

    // Clean up after test
    cleanupUserTests();
    return success;
}

bool TestDatabaseManager::testVerifyUser()
{
    // Clean up and setup
    db.createUser("testuser", "testpass", 123);

    // Perform the test
    bool success = db.verifyUser("testuser", "testpass");

    cleanupUserTests();
    return success;
}

bool TestDatabaseManager::testReadUserByUsername()
{
    // Clean up and setup
    db.createUser("testuser", "testpass", 123);

    // Perform the test
    auto user = db.readUserByUsername("testuser");
    bool success = user.has_value() &&
                   std::get<UserFields::username>(*user) == "testuser";

    // Clean up
    cleanupUserTests();
    return success;
}

bool TestDatabaseManager::testReadSenderCompId()
{
    db.createUser("testuser", "testpass", 123);

    // Perform the test
    auto senderCompId = db.readSenderCompIdByUsername("testuser");
    bool success = senderCompId.has_value() && senderCompId.value() == 123;

    // Clean up
    cleanupUserTests();
    return success;
}

bool TestDatabaseManager::testReadAllUsers()
{
    // Clean up and setup
    cleanupUserTests();
    db.createUser("testuser", "testpass", 123);

    // Perform the test
    auto users = db.readAllUsers();
    bool success = !users.empty();

    // Clean up
    cleanupUserTests();
    return success;
}

bool TestDatabaseManager::testUpdateUser()
{
    // Clean up and setup
    db.createUser("testuser", "testpass", 123);

    // Get the user for update
    auto user = db.readUserByUsername("testuser");
    if (!user.has_value())
        return false;

    // Perform the test
    bool success = db.updateUser(
        std::get<UserFields::id>(*user),
        "newpass",
        456);

    // Clean up
    cleanupUserTests();
    return success;
}

bool TestDatabaseManager::testDeleteUser()
{

    db.createUser("testuser", "testpass", 123);

    // Get the user for deletion
    auto user = db.readUserByUsername("testuser");
    if (!user.has_value())
        return false;

    // Perform the test
    bool success = db.deleteUser(
        std::get<UserFields::id>(*user));

    // No need to clean up as the test itself deletes the user
    return success;
}

////////////////////  END OF USER TABLE TESTS ////////////////////

// Trade Table Tests
bool TestDatabaseManager::testPipeCreateTrade()
{
    try
    {
        // Clean up first
        cleanupTradeTests();
        cleanupOrderTests();
        cleanupUserTests();

        // Create test user
        db.createUser("testuser", "testpass", 123);
        auto user = db.readUserByUsername("testuser");
        if (!user.has_value())
        {
            std::cerr << "Failed to create test user" << std::endl;
            return false;
        }
        int userId = std::get<UserFields::id>(*user);

        // Create test order
        db.startPipe();
        db.pipeCreateOrder(userId, "BTC", "BUY", 50000.0, 1.0);
        if (!db.executePipe())
        {
            std::cerr << "Failed to create test order" << std::endl;
            return false;
        }

        // Get the created order
        auto orders = db.readOrdersByUser(userId);
        if (orders.empty())
        {
            std::cerr << "No orders found for user" << std::endl;
            return false;
        }
        int orderId = std::get<OrderFields::id>(orders[0]);

        // Create trade
        db.startPipe();
        db.pipeCreateTrade(orderId, userId, 0.5);
        bool success = db.executePipe();

        // Clean up
        cleanupTradeTests();
        cleanupOrderTests();
        cleanupUserTests();
        return success;
    }
    catch (const std::exception &e)
    {
        std::cerr << "testPipeCreateTrade failed: " << e.what() << std::endl;
        cleanupTradeTests();
        cleanupOrderTests();
        cleanupUserTests();
        return false;
    }
}

bool TestDatabaseManager::testReadTradesByUser()
{
    try
    {
        // Clean up first
        cleanupTradeTests();
        cleanupOrderTests();
        cleanupUserTests();

        // Create test user
        db.createUser("testuser", "testpass", 123);
        auto user = db.readUserByUsername("testuser");
        if (!user.has_value())
            return false;
        int userId = std::get<UserFields::id>(*user);

        // Create test order
        db.startPipe();
        db.pipeCreateOrder(userId, "BTC", "BUY", 50000.0, 1.0);
        db.executePipe();

        // Get the created order
        auto orders = db.readOrdersByUser(userId);
        if (orders.empty())
            return false;
        int orderId = std::get<OrderFields::id>(orders[0]);

        // Create test trade
        db.startPipe();
        db.pipeCreateTrade(orderId, userId, 0.5);
        db.executePipe();

        // Test reading trades
        auto trades = db.readTradesByUser(userId);
        bool success = !trades.empty();

        // Clean up
        cleanupTradeTests();
        cleanupOrderTests();
        cleanupUserTests();
        return success;
    }
    catch (const std::exception &e)
    {
        std::cerr << "testReadTradesByUser failed: " << e.what() << std::endl;
        cleanupTradeTests();
        cleanupOrderTests();
        cleanupUserTests();
        return false;
    }
}

bool TestDatabaseManager::testReadTradesByOrder()
{
    try
    {
        // Clean up first
        cleanupTradeTests();
        cleanupOrderTests();
        cleanupUserTests();

        // Create test user
        db.createUser("testuser", "testpass", 123);
        auto user = db.readUserByUsername("testuser");
        if (!user.has_value())
            return false;
        int userId = std::get<UserFields::id>(*user);

        // Create test order
        db.startPipe();
        db.pipeCreateOrder(userId, "BTC", "BUY", 50000.0, 1.0);
        db.executePipe();

        // Get the created order
        auto orders = db.readOrdersByUser(userId);
        if (orders.empty())
            return false;
        int orderId = std::get<OrderFields::id>(orders[0]);

        // Create test trade
        db.startPipe();
        db.pipeCreateTrade(orderId, userId, 0.5);
        db.executePipe();

        // Test reading trades by order
        auto trades = db.readTradesByOrder(orderId);
        bool success = !trades.empty();

        // Clean up
        cleanupTradeTests();
        cleanupOrderTests();
        cleanupUserTests();
        return success;
    }
    catch (const std::exception &e)
    {
        std::cerr << "testReadTradesByOrder failed: " << e.what() << std::endl;
        cleanupTradeTests();
        cleanupOrderTests();
        cleanupUserTests();
        return false;
    }
}

////////////////////  END OF TRADE TABLE TESTS ////////////////////
// Order Table Tests
bool TestDatabaseManager::testPipeCreateOrder()
{
    try
    {
        // Clean up first
        cleanupOrderTests();
        cleanupUserTests();

        // Create test user and verify creation

        if (!db.createUser("testuser", "testpass", 12343))
        {
            std::cerr << "Failed to create test user" << std::endl;
            return false;
        }

        // Verify user exists
        auto user = db.readUserByUsername("testuser");
        if (!user.has_value())
        {
            std::cerr << "Created user not found" << std::endl;
            return false;
        }
        auto userId = std::get<UserFields::id>(*user);
        // Create order using the verified user ID
        db.startPipe();
        db.pipeCreateOrder(userId, "BTC", "BUY", 50000.0, 1.0);
        bool success = db.executePipe();

        // Clean up
        cleanupOrderTests();
        cleanupUserTests();
        return success;
    }
    catch (const std::exception &e)
    {
        std::cerr << "testPipeCreateOrder failed: " << e.what() << std::endl;
        cleanupOrderTests();
        cleanupUserTests();
        return false;
    }
}

bool TestDatabaseManager::testReadOrdersByUser()
{
    try
    {
        // Create test user with predefined ID
        db.createUser("testuser", "testpass", 12345);
        // Read the user ID for the created user
        auto createdUser = db.readUserByUsername("testuser");
        if (!createdUser.has_value())
        {
            std::cerr << "Failed to read created user" << std::endl;
            return false;
        }
        int userId = std::get<UserFields::id>(*createdUser);
        // Create test order using the predefined user ID
        db.startPipe();
        db.pipeCreateOrder(userId, "BTC", "BUY", 50000.0, 1.0);
        db.executePipe();

        // Test reading orders using the same ID
        auto orders = db.readOrdersByUser(userId);
        bool success = !orders.empty();

        // Clean up
        cleanupOrderTests();
        cleanupUserTests();
        return success;
    }
    catch (const std::exception &e)
    {
        std::cerr << "testReadOrdersByUser failed: " << e.what() << std::endl;
        cleanupOrderTests();
        cleanupUserTests();
        return false;
    }
}

bool TestDatabaseManager::testReadOrdersBySymbol()
{
    try
    {
        // Create test user and order
        db.createUser("testuser", "testpass", 123);
        auto user = db.readUserByUsername("testuser");
        if (!user.has_value())
            return false;

        db.startPipe();
        db.pipeCreateOrder(std::get<UserFields::id>(*user), "BTC", "BUY", 50000.0, 1.0);
        db.executePipe();

        // Test reading orders by symbol
        auto orders = db.readOrdersBySymbol("BTC");
        bool success = !orders.empty();

        // Clean up
        cleanupOrderTests();
        cleanupUserTests();
        return success;
    }
    catch (const std::exception &e)
    {
        std::cerr << "testReadOrdersBySymbol failed: " << e.what() << std::endl;
        cleanupOrderTests();
        cleanupUserTests();
        return false;
    }
}

bool TestDatabaseManager::testPipeUpdateOrderRemainingQuantity()
{
    try
    {
        // Create test user and order
        db.createUser("testuser", "testpass", 123);
        auto user = db.readUserByUsername("testuser");
        if (!user.has_value())
            return false;

        db.startPipe();
        db.pipeCreateOrder(std::get<UserFields::id>(*user), "BTC", "BUY", 50000.0, 1.0);
        db.executePipe();

        // Get the created order
        auto orders = db.readOrdersByUser(std::get<UserFields::id>(*user));
        if (orders.empty())
        {
            std::cerr << "No orders found for user" << std::endl;
            return false;
        }

        // Update the order
        db.startPipe();
        db.pipeUpdateOrderRemainingQuantity(std::get<OrderFields::id>(orders[0]), 0.5);
        bool success = db.executePipe();

        // Clean up
        cleanupOrderTests();
        cleanupUserTests();
        return success;
    }
    catch (const std::exception &e)
    {
        std::cerr << "testPipeUpdateOrderRemainingQuantity failed: " << e.what() << std::endl;
        cleanupOrderTests();
        cleanupUserTests();
        return false;
    }
}

bool TestDatabaseManager::testPipeUpdateOrderFilledQuantity()
{
    try
    {
        db.startPipe();
        db.pipeUpdateOrderFilledQuantity(1, 0.5);
        return db.executePipe();
    }
    catch (const std::exception &e)
    {
        std::cerr << "testPipeUpdateOrderFilledQuantity failed: " << e.what() << std::endl;
        return false;
    }
}

bool TestDatabaseManager::testPipeDeleteOrder()
{
    try
    {
        db.startPipe();
        db.pipeDeleteOrder(1);
        return db.executePipe();
    }
    catch (const std::exception &e)
    {
        std::cerr << "testPipeDeleteOrder failed: " << e.what() << std::endl;
        return false;
    }
}

////////////////////  END OF ORDER TABLE TESTS ////////////////////

// Balance Table Tests
bool TestDatabaseManager::testCreateBalance()
{
    try
    {
        // Clean up first
        cleanupBalanceTests();
        cleanupUserTests();

        // Create test user
        db.createUser("testuser", "testpass", 123);
        auto user = db.readUserByUsername("testuser");
        if (!user.has_value())
        {
            std::cerr << "Failed to create test user" << std::endl;
            return false;
        }
        int userId = std::get<UserFields::id>(*user);

        // Create balance
        db.createBalance(userId, "BTC", 1.0);

        // Verify creation
        auto balance = db.readBalanceByUserAndAsset(userId, "BTC");
        bool success = balance.has_value() && balance.value() == 1.0;

        // Clean up
        cleanupBalanceTests();
        cleanupUserTests();
        return success;
    }
    catch (const std::exception &e)
    {
        std::cerr << "testCreateBalance failed: " << e.what() << std::endl;
        cleanupBalanceTests();
        cleanupUserTests();
        return false;
    }
}

bool TestDatabaseManager::testReadBalancesByUser()
{
    try
    {
        // Clean up first
        cleanupBalanceTests();
        cleanupUserTests();

        // Create test user
        db.createUser("testuser", "testpass", 123);
        auto user = db.readUserByUsername("testuser");
        if (!user.has_value())
            return false;
        int userId = std::get<UserFields::id>(*user);

        // Create multiple balances for the user
        db.createBalance(userId, "BTC", 1.0);
        db.createBalance(userId, "ETH", 2.0);

        // Test reading balances
        auto balances = db.readBalancesByUser(userId);
        bool success = !balances.empty() && balances.size() == 2;

        // Optional: Verify balance data
        if (success)
        {
            for (const auto &[asset, amount] : balances)
            {
                std::cout << "Asset: " << asset << ", Amount: " << amount << std::endl;
            }
        }

        // Clean up
        cleanupBalanceTests();
        cleanupUserTests();
        return success;
    }
    catch (const std::exception &e)
    {
        std::cerr << "testReadBalancesByUser failed: " << e.what() << std::endl;
        cleanupBalanceTests();
        cleanupUserTests();
        return false;
    }
}

bool TestDatabaseManager::testReadBalanceByUserAndAsset()
{
    try
    {
        // Clean up first
        cleanupBalanceTests();
        cleanupUserTests();

        // Create test user
        db.createUser("testuser", "testpass", 123);
        auto user = db.readUserByUsername("testuser");
        if (!user.has_value())
            return false;
        int userId = std::get<UserFields::id>(*user);

        // Create test balance
        const double initialAmount = 1.0;
        db.createBalance(userId, "BTC", initialAmount);

        // Test reading specific balance
        auto balance = db.readBalanceByUserAndAsset(userId, "BTC");
        bool success = balance.has_value() && balance.value() == initialAmount;

        // Clean up
        cleanupBalanceTests();
        cleanupUserTests();
        return success;
    }
    catch (const std::exception &e)
    {
        std::cerr << "testReadBalanceByUserAndAsset failed: " << e.what() << std::endl;
        cleanupBalanceTests();
        cleanupUserTests();
        return false;
    }
}

bool TestDatabaseManager::testPipeUpdateBalance()
{
    try
    {
        // Clean up first
        cleanupBalanceTests();
        cleanupUserTests();

        // Create test user
        db.createUser("testuser", "testpass", 123);
        auto user = db.readUserByUsername("testuser");
        if (!user.has_value())
            return false;
        int userId = std::get<UserFields::id>(*user);

        // Create initial balance
        const double initialAmount = 1.0;
        const double updatedAmount = 2.0;
        db.createBalance(userId, "BTC", initialAmount);

        // Update balance in transaction
        db.startPipe();
        db.pipeUpdateBalance(userId, "BTC", updatedAmount);
        bool updateSuccess = db.executePipe();

        if (!updateSuccess)
        {
            std::cerr << "Failed to execute update balance pipe" << std::endl;
            return false;
        }

        // Verify the update
        auto newBalance = db.readBalanceByUserAndAsset(userId, "BTC");
        bool success = newBalance.has_value() && newBalance.value() == updatedAmount;

        // Clean up
        cleanupBalanceTests();
        cleanupUserTests();
        return success;
    }
    catch (const std::exception &e)
    {
        std::cerr << "testPipeUpdateBalance failed: " << e.what() << std::endl;
        cleanupBalanceTests();
        cleanupUserTests();
        return false;
    }
}
////////////////////  END OF BALANCE TABLE TESTS ////////////////////
// Transaction Rollback Tests

bool TestDatabaseManager::testTransactionRollback()
{
    try
    {
        // Setup initial balance
        db.createBalance(1, "BTC", 1.0);
        auto initialBalance = db.readBalanceByUserAndAsset(1, "BTC");
        if (!initialBalance.has_value())
        {
            std::cerr << "Failed to setup initial balance" << std::endl;
            return false;
        }

        // Start transaction that should fail
        db.startPipe();

        // First operation (should be rolled back)
        db.pipeUpdateBalance(1, "BTC", 2.0);

        // Second operation (should cause failure)
        try
        {
            db.pipeUpdateBalance(999999, "BTC", -999999); // Invalid operation
            db.executePipe();
            std::cerr << "Transaction should have failed but didn't" << std::endl;
            return false;
        }
        catch (...)
        {
            // Expected failure
            // Now verify the balance wasn't changed (rollback successful)
            auto finalBalance = db.readBalanceByUserAndAsset(1, "BTC");
            if (!finalBalance.has_value())
            {
                std::cerr << "Failed to read final balance" << std::endl;
                return false;
            }

            // Verify balance remained unchanged
            bool rollbackSuccessful = (finalBalance.value() == initialBalance.value());
            if (!rollbackSuccessful)
            {
                std::cerr << "Rollback failed: Balance changed from "
                          << initialBalance.value() << " to "
                          << finalBalance.value() << std::endl;
            }
            return rollbackSuccessful;
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "testTransactionRollback failed: " << e.what() << std::endl;
        return false;
    }
}

void TestDatabaseManager::runUserTableTests()
{
    std::cout << "\n=== Running User Table Tests ===\n";

    // Create user first as other tests depend on it
    bool createSuccess = testCreateUser();
    printTestResult("Create User Test", createSuccess);
    if (!createSuccess)
    {
        std::cout << "User creation failed, skipping remaining tests\n";
        return;
    }

    // Run verification test
    bool verifySuccess = testVerifyUser();
    printTestResult("Verify User Test", verifySuccess);

    // Run read operations
    bool readUserSuccess = testReadUserByUsername();
    printTestResult("Read User By Username Test", readUserSuccess);

    bool readSenderSuccess = testReadSenderCompId();
    printTestResult("Read SenderCompId Test", readSenderSuccess);

    bool readAllSuccess = testReadAllUsers();
    printTestResult("Read All Users Test", readAllSuccess);

    // Run update operation
    bool updateSuccess = testUpdateUser();
    printTestResult("Update User Test", updateSuccess);

    // Run delete operation last as it removes the test user
    bool deleteSuccess = testDeleteUser();
    printTestResult("Delete User Test", deleteSuccess);

    std::cout << "=== Completed: User Table Tests ===\n";
}

void TestDatabaseManager::cleanupUserTests()
{
    try
    {
        // Start a transaction for cleanup
        pqxx::work txn(db.getConnection()); // You'll need to add a getter in DatabaseManager
        txn.exec("DELETE FROM users;");     // Delete all records from users table
        txn.commit();

        // Verify the table is empty
        auto allUsers = db.readAllUsers();
        if (!allUsers.empty())
        {
            std::cerr << "Warning: User table cleanup may not have been completely successful" << std::endl;
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Cleanup failed: " << e.what() << std::endl;
    }
}

void TestDatabaseManager::cleanupOrderTests()
{
    try
    {
        // Make sure any existing transaction is completed
        if (db.isPipeActive())
        {
            db.executePipe();
        }

        // Start a transaction for cleanup
        pqxx::work txn(db.getConnection());
        txn.exec("DELETE FROM orders;"); // Delete all records from orders table
        txn.commit();

        // Verify the table is empty
        auto allOrders = db.readOrdersBySymbol("BTC"); // Using BTC as test symbol
        if (!allOrders.empty())
        {
            std::cerr << "Warning: Order table cleanup may not have been completely successful" << std::endl;
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Order cleanup failed: " << e.what() << std::endl;
    }
}

void TestDatabaseManager::runOrderTableTests()
{
    std::cout << "\n=== Running Order Table Tests ===\n";

    // Clean up any existing orders
    cleanupOrderTests();

    // Test order creation
    bool createOrderSuccess = testPipeCreateOrder();
    printTestResult("Create Order Test", createOrderSuccess);
    if (!createOrderSuccess)
    {
        std::cout << "Order creation failed, skipping remaining tests\n";
        return;
    }

    // Test read operations
    bool readByUserSuccess = testReadOrdersByUser();
    printTestResult("Read Orders By User Test", readByUserSuccess);

    bool readBySymbolSuccess = testReadOrdersBySymbol();
    printTestResult("Read Orders By Symbol Test", readBySymbolSuccess);

    // Test update operations
    bool updateRemainingQtySuccess = testPipeUpdateOrderRemainingQuantity();
    printTestResult("Update Order Remaining Quantity Test", updateRemainingQtySuccess);

    bool updateFilledQtySuccess = testPipeUpdateOrderFilledQuantity();
    printTestResult("Update Order Filled Quantity Test", updateFilledQtySuccess);

    // Test delete operation last
    bool deleteSuccess = testPipeDeleteOrder();
    printTestResult("Delete Order Test", deleteSuccess);

    std::cout << "=== Completed: Order Table Tests ===\n";

    // Final cleanup
    cleanupOrderTests();
}

void TestDatabaseManager::cleanupTradeTests()
{
    try
    {
        // Make sure any existing transaction is completed
        if (db.isPipeActive())
        {
            db.executePipe();
        }

        // Start a transaction for cleanup
        pqxx::work txn(db.getConnection());
        txn.exec("DELETE FROM trades;"); // Delete all records from trades table
        txn.commit();

        // Verify the table is empty
        auto allTrades = db.readTradesByUser(1); // Using user_id 1 as test
        if (!allTrades.empty())
        {
            std::cerr << "Warning: Trade table cleanup may not have been completely successful" << std::endl;
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Trade cleanup failed: " << e.what() << std::endl;
    }
}

void TestDatabaseManager::runTradeTableTests()
{
    std::cout << "\n=== Running Trade Table Tests ===\n";

    // Clean up any existing data
    cleanupTradeTests();
    cleanupOrderTests();
    cleanupUserTests();

    // Test trade creation
    bool createTradeSuccess = testPipeCreateTrade();
    printTestResult("Create Trade Test", createTradeSuccess);
    if (!createTradeSuccess)
    {
        std::cout << "Trade creation failed, skipping remaining tests\n";
        return;
    }

    // Test read operations
    bool readByUserSuccess = testReadTradesByUser();
    printTestResult("Read Trades By User Test", readByUserSuccess);

    bool readByOrderSuccess = testReadTradesByOrder();
    printTestResult("Read Trades By Order Test", readByOrderSuccess);

    std::cout << "=== Completed: Trade Table Tests ===\n";

    // Final cleanup
    cleanupTradeTests();
    cleanupOrderTests();
    cleanupUserTests();
}

void TestDatabaseManager::cleanupBalanceTests()
{
    try
    {
        // Make sure any existing transaction is completed
        if (db.isPipeActive())
        {
            db.executePipe();
        }

        // Start a transaction for cleanup
        pqxx::work txn(db.getConnection());
        txn.exec("DELETE FROM balances;"); // Delete all records from balances table
        txn.commit();

        // Verify the table is empty
        auto testBalances = db.readBalancesByUser(1); // Using user_id 1 as test
        if (!testBalances.empty())
        {
            std::cerr << "Warning: Balance table cleanup may not have been completely successful" << std::endl;
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Balance cleanup failed: " << e.what() << std::endl;
    }
}
void TestDatabaseManager::runBalanceTableTests()
{
    std::cout << "\n=== Running Balance Table Tests ===\n";

    // Clean up any existing data
    cleanupBalanceTests();
    cleanupUserTests();

    // Test balance creation
    bool createBalanceSuccess = testCreateBalance();
    printTestResult("Create Balance Test", createBalanceSuccess);
    if (!createBalanceSuccess)
    {
        std::cout << "Balance creation failed, skipping remaining tests\n";
        return;
    }

    // Test read operations
    bool readBalancesByUserSuccess = testReadBalancesByUser();
    printTestResult("Read Balances By User Test", readBalancesByUserSuccess);

    bool readBalanceByUserAndAssetSuccess = testReadBalanceByUserAndAsset();
    printTestResult("Read Balance By User And Asset Test", readBalanceByUserAndAssetSuccess);

    // Test update operation
    bool updateBalanceSuccess = testPipeUpdateBalance();
    printTestResult("Update Balance Test", updateBalanceSuccess);

    std::cout << "=== Completed: Balance Table Tests ===\n";

    // Final cleanup
    cleanupBalanceTests();
    cleanupUserTests();
}
void TestDatabaseManager::runAllTests()
{
    std::cout << "\n=== Starting Database Tests ===\n";

    // Run transaction management tests
    runTransactionManagementTests();

    // Run table-specific tests
    runUserTableTests();

    // Run order table tests
    runOrderTableTests();

    // Run trade table tests
    runTradeTableTests();

    // You can add more test suites here
    // runTradeTableTests();
    // runBalanceTableTests();
    runBalanceTableTests();
    std::cout << "\n=== Database Tests Complete ===\n";
}
