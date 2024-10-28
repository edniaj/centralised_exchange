// MatchingEngine.h
#pragma once

#include <string>
#include <map>
#include <unordered_map>
#include <queue>
#include <set>
#include "FixMessage.h"
#include "RedisManager.h"

class MatchingEngine {
private:
    // Identify which symbols this instance handles
    std::string engine_id;
    std::set<std::string> assigned_symbols;
    RedisManager& redisManager;

    struct Order {
        std::string orderId;
        std::string symbol;
        std::string senderCompId;
        double price;
        int quantity;
        bool isBuy;
        std::chrono::system_clock::time_point timestamp;
    };

    // Local orderbooks (in-memory for speed)
    std::unordered_map<std::string, std::map<double, std::queue<Order>>> buyOrders;    // symbol -> price -> orders
    std::unordered_map<std::string, std::map<double, std::queue<Order>>> sellOrders;   // symbol -> price -> orders
    
    // Track active orders
    std::unordered_map<std::string, Order> activeOrders;  // orderId -> Order

    // Private methods
    void updateRedis(const std::string& symbol);  // Async update to Redis after matching
    bool canProcessSymbol(const std::string& symbol) const;  // Check if we own this symbol

public:
    MatchingEngine(const std::string& engineId, RedisManager& redis);

    // Core matching functionality
    bool processOrder(const FIXMessage& fixMsg);
    bool cancelOrder(const FIXMessage& fixMsg);
    bool modifyOrder(const FIXMessage& fixMsg);
    bool processMarketOrder(const FIXMessage& fixMsg);

    // Symbol management
    void addSymbol(const std::string& symbol);
    void removeSymbol(const std::string& symbol);
    bool hasSymbol(const std::string& symbol) const;

    // State management
    void loadState();  // Load state from Redis on startup
    void saveState();  // Save state to Redis (periodic/shutdown)
};