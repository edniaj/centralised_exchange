#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <sw/redis++/redis++.h>
#include "RedisManager.h"

// Helper function implementation
std::string RedisManager::createConnectionString(const std::string &uri, const std::string &symbol, const std::unordered_map<std::string, int> &symbolToDbMap)
{
    auto it = symbolToDbMap.find(symbol);
    if (it == symbolToDbMap.end())
    {
        throw std::runtime_error("Symbol not configured: " + symbol);
    }
    int dbNumber = it->second;
    return uri + "/" + std::to_string(dbNumber);
}

// Constructor implementation
RedisManager::RedisManager(const std::string &uri, const std::string &symbol)
    : symbolToDb{
          {"AAPL", 0},
          {"BTC", 1},
          {"ETH", 2},
          {"TESTING", 15}},
      redis(createConnectionString(uri, symbol, symbolToDb)), pipe(redis.pipeline())
{
    // No additional logic needed here
}

std::string RedisManager::ping()
{
    return redis.ping();
}

std::vector<std::string> RedisManager::getAllKeys()
{
    try
    {
        std::vector<std::string> keys;
        redis.keys("*", std::back_inserter(keys));
        return keys;
    }
    catch (const sw::redis::Error &e)
    {
        throw std::runtime_error("Failed to get keys: " + std::string(e.what()));
    }
}

// Pipe related operations to CRUD
bool RedisManager::pipeCreateOrder(const std::string &orderId, const std::string &userId, const std::string &side,
                                   double price, int quantity)
{
    try
    {

        std::vector<std::pair<std::string, std::string>> field_values = {
            {"order_id", orderId},
            {"user_id", userId},
            {"side", side},
            {"price", std::to_string(price)},
            {"quantity", std::to_string(quantity)},
            {"timestamp", std::to_string(std::time(nullptr))}};

        pipe.hmset("orders:" + orderId, field_values.begin(), field_values.end());
        pipe.sadd("user:" + userId + ":orders", orderId);

        return true;
    }
    catch (const sw::redis::Error &e)
    {
        std::cerr << "RedisManager::pipeCreateOrder error " << e.what() << std::endl;
        return false;
    }
}
bool RedisManager::pipeUpdateOrder(const std::string &order_id,
                                   const std::unordered_map<std::string, std::string> &updates)
{
    try
    {
        // Validate that only price and quantity are being updated
        for (const auto &update : updates)
        {
            if (update.first != "price" && update.first != "quantity")
            {
                std::cerr << "Invalid update field: " << update.first << ". Only price and quantity updates are allowed." << std::endl;
                return false;
            }

            // Validate price is a valid double
            if (update.first == "price")
            {
                try
                {
                    double price = std::stod(update.second);
                    if (price <= 0.0)
                    {
                        std::cerr << "Invalid price value: Price must be positive" << std::endl;
                        return false;
                    }
                }
                catch (const std::invalid_argument &e)
                {
                    std::cerr << "Invalid price format: Must be a valid number" << std::endl;
                    return false;
                }
                catch (const std::out_of_range &e)
                {
                    std::cerr << "Price value out of range" << std::endl;
                    return false;
                }
            }

            // Validate quantity is a valid integer
            if (update.first == "quantity")
            {
                try
                {
                    int quantity = std::stoi(update.second);
                    if (quantity <= 0)
                    {
                        return false;
                    }
                }
                catch (const std::invalid_argument &e)
                {
                    return false;
                }
                catch (const std::out_of_range &e)
                {

                    return false;
                }
            }
        }

        // If validation passes, proceed with the update
        pipe.hmset("orders:" + order_id, updates.begin(), updates.end());

        // If price was updated, add timestamp in a separate command
        if (updates.find("price") != updates.end())
        {
            pipe.hset("orders:" + order_id, "timestamp", std::to_string(std::time(nullptr)));
        }

        return true;
    }
    catch (const sw::redis::Error &e)
    {
        std::cerr << "Redis error in pipeUpdateOrder: " << e.what() << std::endl;
        return false;
    }
}

bool RedisManager::pipeDeleteOrder(const std::string &order_id)
{
    try
    {
        std::unordered_map<std::string, std::string> orderDetails;
        redis.hgetall("orders:" + order_id, std::inserter(orderDetails, orderDetails.begin()));

        if (orderDetails.empty())
        {
            std::cerr << "Order " << order_id << " not found" << std::endl;
            return false;
        }

        std::string userId = orderDetails["user_id"];

        pipe.del("orders:" + order_id);
        pipe.srem("user:" + userId + ":orders", order_id);

        return true;
    }
    catch (const sw::redis::Error &e)
    {
        std::cerr << "Redis error in pipeDeleteOrder: " << e.what() << std::endl;
        return false;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error in pipeDeleteOrder: " << e.what() << std::endl;
        return false;
    }
}
// And here's pipeExecute() if you need it:
bool RedisManager::pipeExecute()
{
    try
    {
        pipe.exec();
        return true;
    }
    catch (const sw::redis::Error &e)
    {
        std::cerr << "Redis error in pipeExecute: " << e.what() << std::endl;
        return false;
    }
}