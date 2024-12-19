#include "../include/DatabaseManager.h"
#include <iostream>
#include <optional>


DatabaseManager::DatabaseManager(const std::string &connString) : conn(connString)
{
    try
    {
        pqxx::work txn(conn); // Same as writing pqxx:work txn = pqxx::work(conn);
        pqxx::result result = txn.exec("SELECT COUNT(*) FROM users");
        txn.commit(); // Releases Locks on the rows that were read.

        if (!result.empty())
        {
            int userCount = result[0][0].as<int>(); // as<Type>() is a pqxx template method that converts the value to the specified type.
            std::cout << "\n=============================\nDatabase contains " << userCount << " user(s)." << std::endl;
        }
        else
        {
            std::cout << "No user found" << std::endl;
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error counting users: " << e.what() << std::endl;
    }
}

// Transaction Management
bool DatabaseManager::startPipe()
{
    txn = std::make_unique<pqxx::work>(conn);
    return true;
}

bool DatabaseManager::executePipe()
{
    txn->commit();
    txn.reset();
    return true;
}

////////////////////  END OF TRANSACTION MANAGEMENT ////////////////////

// USER Table Methods
bool DatabaseManager::verifyUser(const std::string &username, const std::string &password)
{
    try
    {
        pqxx::work txn(conn);
        pqxx::result result = txn.exec_params(
            "SELECT password FROM users WHERE username = $1",
            username);

        if (result.empty())
        {
            return false; // User not found
        }

        std::string storedPassword = result[0][0].as<std::string>();
        txn.commit();

        // Compare the stored password with the provided password
        return (storedPassword == password);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error verifying user: " << e.what() << std::endl;
        return false;
    }
}

// CREATE - Insert a new user
bool DatabaseManager::createUser(const std::string &username, const std::string &password,
                                 int senderCompId)
{
    try
    {
        pqxx::work txn(conn);

        pqxx::result result = txn.exec_params(
            "INSERT INTO users (username, password, sendercompid, created_at) "
            "VALUES ($1, $2, $3, CURRENT_TIMESTAMP) RETURNING id",
            username, password, senderCompId);

        txn.commit();
        return !result.empty();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error creating user: " << e.what() << std::endl;
        return false;
    }
}

// READ - 1. Read all users | 2. Read user by username | 3. Read user by senderCompId

std::vector<UserData> DatabaseManager::readAllUsers()
{
    std::vector<UserData> users;
    try
    {
        pqxx::work txn(conn);
        pqxx::result result = txn.exec(
            "SELECT id, username, password, sendercompid, created_at "
            "FROM users ORDER BY id");
        txn.commit();

        for (const auto &row : result)
        {
            users.push_back(std::make_tuple(
                row[0].as<int>(),         // id
                row[1].as<std::string>(), // username
                row[2].as<std::string>(), // password
                row[3].as<int>(),         // sendercompid
                row[4].as<std::string>()  // created_at
            ));
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error fetching all users: " << e.what() << std::endl;
    }
    return users;
}
// READ - Get user by username
std::optional<UserData> DatabaseManager::readUserByUsername(const std::string &username)
{
    try
    {
        pqxx::work txn(conn);
        pqxx::result result = txn.exec_params(
            "SELECT id, username, password, sendercompid, created_at "
            "FROM users WHERE username = $1",
            username);
        txn.commit();

        if (result.empty())
        {
            return std::nullopt;
        }

        return std::make_tuple(
            result[0][0].as<int>(),         // id
            result[0][1].as<std::string>(), // username
            result[0][2].as<std::string>(), // password
            result[0][3].as<int>(),         // sendercompid
            result[0][4].as<std::string>()  // created_at
        );
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error fetching user: " << e.what() << std::endl;
        return std::nullopt;
    }
}

std::optional<int> DatabaseManager::readSenderCompIdByUsername(const std::string &username)
{
    try
    {
        pqxx::work txn(conn);
        pqxx::result result = txn.exec_params(
            "SELECT sendercompid FROM users WHERE username = $1",
            username);

        if (result.empty())
        {
            return std::nullopt; // User not found, return nullopt
        }

        txn.commit();
        return result[0][0].as<int>();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error fetching user sendercompid: " << e.what() << std::endl;
        return std::nullopt; // Return nullopt on error
    }
}
// UPDATE - Update user details
bool DatabaseManager::updateUser(int userId, const std::string &newPassword,
                                 int newSenderCompId)
{
    try
    {
        pqxx::work txn(conn);

        pqxx::result result = txn.exec_params(
            "UPDATE users "
            "SET password = $1, sendercompid = $2 "
            "WHERE id = $3 "
            "RETURNING id",
            newPassword, newSenderCompId, userId);

        txn.commit();
        return !result.empty();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error updating user: " << e.what() << std::endl;
        return false;
    }
}
// DELETE - Delete a user
bool DatabaseManager::deleteUser(int userId)
{
    try
    {
        pqxx::work txn(conn);

        pqxx::result result = txn.exec_params(
            "DELETE FROM users WHERE id = $1 RETURNING id",
            userId);

        txn.commit();
        return !result.empty();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error deleting user: " << e.what() << std::endl;
        return false;
    }
}
////////////////////  END OF USER TABLE METHODS ////////////////////

// Order Table Methods
void DatabaseManager::pipeCreateOrder(int userId, const std::string &symbol, const std::string &side,
                                      double price, double remainingQty)
{
    if (!txn)
        throw std::runtime_error("No active transaction");

    txn->exec_params(
        "INSERT INTO orders (user_id, symbol, side, price, remaining_quantity, filled_quantity, created_at) "
        "VALUES ($1, $2, $3, $4, $5, $6, NOW())",
        userId, symbol, side, price, remainingQty, 0.0);
}

// Read Operations (not piped as they're queries)
std::vector<OrderData> DatabaseManager::readOrdersByUser(int userId)
{
    std::vector<OrderData> orders;
    pqxx::work w(conn);

    auto result = w.exec_params(
        "SELECT id, symbol, side, price, remaining_quantity, filled_quantity, created_at "
        "FROM orders WHERE user_id = $1 ORDER BY created_at DESC",
        userId);

    for (const auto &row : result)
    {
        orders.emplace_back( // emplace_back is used to add an element to the end of the vector - more efficient than push_back
            row[0].as<int>(),         // id
            row[1].as<std::string>(), // symbol
            row[2].as<std::string>(), // side
            row[3].as<double>(),      // price
            row[4].as<double>(),      // remaining_quantity
            row[5].as<double>(),      // filled_quantity
            row[6].as<std::string>()  // created_at
        );
    }
    w.commit(); // query has already been executed. There might be shared locks releases the lock
    return orders;
}
// Will be used in initializaing the matching engine
std::vector<OrderData> DatabaseManager::readOrdersBySymbol(const std::string &symbol)
{
    std::vector<OrderData> orders;
    pqxx::work w(conn);

    auto result = w.exec_params(
        "SELECT id, symbol, side, price, remaining_quantity, filled_quantity, created_at "
        "FROM orders WHERE symbol = $1 ORDER BY price ASC",
        symbol);

    for (const auto &row : result)
    {
        orders.emplace_back(
            row[0].as<int>(),         // id
            row[1].as<std::string>(), // symbol
            row[2].as<std::string>(), // side
            row[3].as<double>(),      // price
            row[4].as<double>(),      // remaining_quantity
            row[5].as<double>(),      // filled_quantity
            row[6].as<std::string>()  // created_at
        );
    }
    w.commit();
    return orders;
}

// Update Operations (piped)
void DatabaseManager::pipeUpdateOrderRemainingQuantity(int orderId, double remainingQty)
{
    if (!txn)
        throw std::runtime_error("No active transaction");
    txn->exec_params(
        "UPDATE orders SET remaining_quantity = $1 WHERE id = $2",
        remainingQty, orderId);
}

void DatabaseManager::pipeUpdateOrderFilledQuantity(int orderId, double filledQty)
{
    if (!txn)
        throw std::runtime_error("No active transaction");
    txn->exec_params(
        "UPDATE orders SET filled_quantity = $1 WHERE id = $2",
        filledQty, orderId);
}

void DatabaseManager::pipeDeleteOrder(int orderId)
{
    if (!txn)
        throw std::runtime_error("No active transaction");
    txn->exec_params("DELETE FROM orders WHERE id = $1", orderId);
}
////////////////////  END OF ORDER TABLE METHODS ////////////////////

// Trades Table Methods
void DatabaseManager::pipeCreateTrade(int makerOrderId, int takerUserId, double quantity)
{
    if (!txn)
        throw std::runtime_error("No active transaction");
    txn->exec_params(
        "INSERT INTO trades (maker_order_id, taker_user_id, quantity, created_at) "
        "VALUES ($1, $2, $3, NOW())",
        makerOrderId, takerUserId, quantity);
}

// Read Operations (not piped)
std::vector<TradeData> DatabaseManager::readTradesByUser(int userId)
{
    std::vector<TradeData> trades;
    pqxx::work w(conn);

    auto result = w.exec_params(
        "SELECT id, maker_order_id, taker_user_id, quantity, created_at "
        "FROM trades WHERE taker_user_id = $1 OR "
        "maker_order_id IN (SELECT id FROM orders WHERE user_id = $1) "
        "ORDER BY created_at DESC",
        userId);

    for (const auto &row : result)
    {
        trades.emplace_back(
            row[0].as<int>(),        // id
            row[1].as<int>(),        // maker_order_id
            row[2].as<int>(),        // taker_user_id
            row[3].as<double>(),     // quantity
            row[4].as<std::string>() // created_at
        );
    }
    w.commit();
    return trades;
}

std::vector<TradeData> DatabaseManager::readTradesByOrder(int orderId)
{
    std::vector<TradeData> trades;
    pqxx::work w(conn);

    auto result = w.exec_params(
        "SELECT id, maker_order_id, taker_user_id, quantity, created_at "
        "FROM trades WHERE maker_order_id = $1 ORDER BY created_at DESC",
        orderId);

    for (const auto &row : result)
    {
        trades.emplace_back(
            row[0].as<int>(),        // id
            row[1].as<int>(),        // maker_order_id
            row[2].as<int>(),        // taker_user_id
            row[3].as<double>(),     // quantity
            row[4].as<std::string>() // created_at
        );
    }
    w.commit();
    return trades;
}
////////////////////  END OF TRADE TABLE METHODS ////////////////////

// Balances Table Methods
void DatabaseManager::createBalance(int userId, const std::string &asset, double amount)
{
    pqxx::work w(conn);
    w.exec_params(
        "INSERT INTO balances (user_id, asset, amount) VALUES ($1, $2, $3)",
        userId, asset, amount);
    w.commit();
}

std::vector<std::tuple<std::string, double>> DatabaseManager::readBalancesByUser(int userId)
{
    std::vector<std::tuple<std::string, double>> balances;
    pqxx::work w(conn);

    auto result = w.exec_params(
        "SELECT asset, amount FROM balances WHERE user_id = $1",
        userId);

    for (const auto &row : result)
    {
        balances.emplace_back(
            row[0].as<std::string>(), // asset
            row[1].as<double>()       // amount
        );
    }
    w.commit();
    return balances;
}

std::optional<double> DatabaseManager::readBalanceByUserAndAsset(int userId, const std::string &asset)
{
    pqxx::work w(conn);

    auto result = w.exec_params(
        "SELECT amount FROM balances WHERE user_id = $1 AND asset = $2",
        userId, asset);

    if (result.empty())
    {
        return std::nullopt;
    }

    double amount = result[0][0].as<double>();
    w.commit();
    return amount;
}

std::vector<std::tuple<int, double>> DatabaseManager::readAllBalancesBySymbol(const std::string &symbol)
{
    std::vector<std::tuple<int, double>> balances;
    pqxx::work w(conn);

    auto result = w.exec_params(
        "SELECT user_id, amount FROM balances WHERE asset = $1 ORDER BY user_id",
        symbol);

    for (const auto &row : result)
    {
        balances.emplace_back(
            row[0].as<int>(),   // user_id
            row[1].as<double>() // amount
        );
    }
    w.commit();
    return balances;
}

void DatabaseManager::pipeUpdateBalance(int userId, const std::string &asset, double newAmount)
{
    if (!txn)
        throw std::runtime_error("No active transaction");
    txn->exec_params(
        "UPDATE balances SET amount = $1 WHERE user_id = $2 AND asset = $3",
        newAmount, userId, asset);
}
////////////////////  END OF BALANCES TABLE METHODS ////////////////////
void DatabaseManager::abortPipe()
{
    if (txn)
    {
        try {
            txn->abort();  // Rollback the transaction
        } catch (...) {
            // Silently handle any errors during abort
        }
        txn.reset();  // Clear the transaction pointer
    }
}
