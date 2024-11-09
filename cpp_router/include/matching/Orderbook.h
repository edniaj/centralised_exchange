#pragma once

#include <string>
#include <map>
#include <set>
#include <unordered_map>
#include <vector>
#include <cstdint>

class Orderbook {
public:
    struct Order {
        std::string order_id;
        std::string user_id;
        std::string side;
        double price;
        int quantity;
        int64_t timestamp;
        std::string status;
    };

    struct PriceLevelInfo {
        int total_quantity;
        int order_count;
    };

    Orderbook(const std::string& symbol);

    void addOrder(const Order& order);
    void removeOrder(const std::string& order_id);
    void modifyOrder(const std::string& order_id, int new_quantity);

    std::vector<Order> getOrdersAtPrice(const std::string& side, double price) const;
    PriceLevelInfo getPriceLevelInfo(const std::string& side, double price) const;
    std::vector<std::string> getUserOrders(const std::string& user_id) const;

private:
    std::string symbol;
    std::map<double, std::set<std::pair<int64_t, std::string>>> buy_orders;
    std::map<double, std::set<std::pair<int64_t, std::string>>> sell_orders;
    std::unordered_map<std::string, Order> order_details;
    std::unordered_map<std::string, PriceLevelInfo> price_level_info;
    std::unordered_map<std::string, std::set<std::string>> user_orders;
};

