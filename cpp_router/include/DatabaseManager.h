// DatabaseManager.h
#ifndef DATABASE_MANAGER_H
#define DATABASE_MANAGER_H

#include <string>
#include <vector>
#include <optional>
#include <pqxx/pqxx>

// it needs to be constexpr so that we can use it in the tuple indices 
namespace UserFields {
    constexpr size_t id = 0;
    constexpr size_t username = 1;
    constexpr size_t password = 2;
    constexpr size_t sender_comp_id = 3;
    constexpr size_t created_at = 4;
}
namespace OrderFields {
    constexpr size_t id = 0;
    constexpr size_t user_id = 1;
    constexpr size_t symbol = 2;
    constexpr size_t side = 3;
    constexpr size_t price = 4;
    constexpr size_t remaining_quantity = 5;
    constexpr size_t filled_quantity = 6;
    constexpr size_t created_at = 7;
}

namespace TradeFields {
    constexpr size_t id = 0;
    constexpr size_t order_id = 1;
    constexpr size_t counterparty_order_id = 2;
    constexpr size_t quantity = 3;
    constexpr size_t created_at = 4;
}

namespace BalanceFields {
    constexpr size_t id = 0;
    constexpr size_t user_id = 1;
    constexpr size_t asset = 2;
    constexpr size_t amount = 3;
    constexpr size_t updated_at = 4;
}

using UserData = std::tuple<int, std::string, std::string, int, std::string>;
using OrderData = std::tuple<int, std::string, std::string, double, double, double, std::string>;
using TradeData = std::tuple<int, int, int, double, std::string>;

class DatabaseManager
{
private:
    pqxx::connection conn;
    // Transaction management
    std::unique_ptr<pqxx::work> txn;

public:
    // Constructor
    DatabaseManager(const std::string &connString);
    // Connection status
    bool isConnected() const { return conn.is_open(); }
    pqxx::connection& getConnection() { return conn; }

    // Transaction Management
    bool startPipe();
    bool executePipe();
    bool isPipeActive() const { return txn != nullptr; }

    // USER Table Methods  - Methods for Users should not be piped, it should be instant since its not part of matching engine
    // Authentication
    bool verifyUser(const std::string &username, const std::string &password);

    // CRUD Operations
    bool createUser(const std::string &username, const std::string &password,
                    int senderCompId);

    std::vector<UserData> readAllUsers();
    std::optional<UserData> readUserByUsername(const std::string &username);
    std::optional<int> readSenderCompIdByUsername(const std::string &username);

    bool updateUser(int userId, const std::string &newPassword,
                    int newSenderCompId);

    bool deleteUser(int userId);

    // Orders Table Methods (piped)
    void pipeCreateOrder(int userId, const std::string &symbol, const std::string &side,
                         double price, double remainingQty);

    // Read Operations (not piped as they're queries)
    std::vector<OrderData> readOrdersByUser(int userId);
    std::vector<OrderData> readOrdersBySymbol(const std::string &symbol);

    // Update Operations (piped)
    void pipeUpdateOrderRemainingQuantity(int orderId, double remainingQty);
    void pipeUpdateOrderFilledQuantity(int orderId, double filledQty);
    void pipeDeleteOrder(int orderId);

    // Trades Table Methods
    void pipeCreateTrade(int makerOrderId, int takerUserId, double quantity);

    // Read Operations (not piped)
    std::vector<TradeData> readTradesByUser(int userId);
    std::vector<TradeData> readTradesByOrder(int orderId);

    // Balances Table Methods
    // This should not be piped as its not part of matching engine
    void createBalance(int userId, const std::string &asset, double amount);

    // Read Operations (not piped)
    std::vector<std::tuple<std::string, double>> readBalancesByUser(int userId);
    std::optional<double> readBalanceByUserAndAsset(int userId, const std::string &asset);
    std::vector<std::tuple<int, double>> readAllBalancesBySymbol(const std::string &symbol);

    // Update Operations (piped)
    void pipeUpdateBalance(int userId, const std::string &asset, double newAmount);

};

#endif // DATABASE_MANAGER_H