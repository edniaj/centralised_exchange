// It is fine to use a serialization / deserialization since Redis is back-up persistent storage.
// THERE IS NO NEED FOR ULTRA OPTIMIZATION HERE. WE WANT THIS TO BE READABLE AND MAINTAINABLE.
// AS LONG AS WE CAN FINISH WRITING AND EXECUTE THE PIPELINE IN A REASONABLE TIME.
// We use Redis because we want to write fast and make it less expensive. Memory > STORAGE

// Redis by default have 16 databases, u can add more if needed by editing the edis.conf:
// RedisManager.h
#ifndef REDISMANAGER_H
#define REDISMANAGER_H

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <stdexcept>
#include <ctime>
#include <sw/redis++/redis++.h>

class RedisManager
{
public:
    // Constructor
    RedisManager(const std::string &uri, const std::string &symbol);

    // Basic Operations
    std::string ping();

    // Pipe related operations to CRUD
    bool pipeCreateOrder(const std::string &orderId,
                         const std::string &userId,
                         const std::string &side,
                         double price,
                         int quantity);

    bool pipeUpdateOrder(const std::string &order_id,
                         const std::unordered_map<std::string, std::string> &updates);

    bool pipeDeleteOrder(const std::string &order_id);

    bool pipeExecute();
    std::vector<std::string> getAllKeys();

private:
    std::unordered_map<std::string, int> symbolToDb; // Map symbols to database numbers
    sw::redis::Redis redis;                          // Redis client instance
    sw::redis::Pipeline pipe;                        // Pipeline instance

    // Helper function to create Redis connection string
    static std::string createConnectionString(const std::string &uri, const std::string &symbol, const std::unordered_map<std::string, int> &symbolToDbMap);
};

#endif // REDISMANAGER_H
