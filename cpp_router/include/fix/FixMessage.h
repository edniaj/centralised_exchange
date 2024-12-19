// FixMessage.h
#pragma once

#include <unordered_map>
#include <vector>
#include <string>
#include <chrono>
#include <cstdint>

class FIXMessage
{
private:
    std::unordered_map<int, std::string> fields; // Map of tags to their corresponding values
    std::vector<int> fieldOrder; // Vector of tags in the order they appear in the message

public:
    FIXMessage(const std::string &message);

    void parse(const std::string &message); // Gotta parse it first before we can read the fields

    std::string getField(int tag) const; // Get the value of a specific tag

    void print() const; // Print the fields in the order they appear

    // Static Method to Create RESPONSE MESSAGES

    static std::string createLogonResponse(const std::string &senderCompId, const std::string &targetCompId);
    static std::string createLogoutResponse(const std::string &senderCompId, const std::string &targetCompId);

};



struct FixBinaryMessage {
    // HEADER - Always Present
    uint64_t timestamp;     // 8 bytes
    // Range: Unix timestamp (seconds since epoch)
    // Example: 1692101234

    uint32_t seqNum;        // 4 bytes (FIX seq num)
    // Range: 1 to 4,294,967,295
    // Typically cycles back to 1 after reaching max

    uint32_t senderCompId;  // 4 bytes
    // Range: 1 to 4,294,967,295
    // Matches your DB sendercompid

    uint32_t targetCompId;  // 4 bytes
    // Range: 1 to 4,294,967,295
    // Usually 1 for exchange

    uint8_t msgType;        // 1 byte
    // Values:
    // 'A' = Logon
    // '5' = Logout
    // 'D' = New Order
    // '8' = Execution Report
    // 'F' = Cancel

    // ORDER FIELDS
    char clOrderId[36];     // 36 bytes
    // UUID string format
    // Example: "123e4567-e89b-12d3-a456-426614174000"

    char symbol[8];         // 8 bytes
    // ASCII string, right-padded with spaces
    // Example: "AAPL    "

    uint8_t side;           // 1 byte
    // Values:
    // '1' = Buy
    // '2' = Sell

    uint8_t ordType;        // 1 byte
    // Values:
    // '1' = Market
    // '2' = Limit

    uint64_t price;         // 8 bytes
    // Range: 0 to 18,446,744,073,709,551,615
    // Stored as price * 10^8
    // Example: 150.50 stored as 15050000000

    uint64_t quantity;      // 8 bytes
    // Range: 0 to 18,446,744,073,709,551,615
    // Stored as quantity * 10^8
    // Example: 100.0 stored as 10000000000

    // EXECUTION REPORT FIELDS
    uint8_t execType;       // 1 byte
    // Values:
    // '0' = New
    // '4' = Canceled
    // 'F' = Trade

    uint8_t ordStatus;      // 1 byte
    // Values:
    // '0' = New
    // '1' = Partially Filled
    // '2' = Filled
    // '4' = Canceled

    uint64_t leavesQty;     // 8 bytes
    // Range: Same as quantity
    // Remaining quantity * 10^8

    uint64_t cumQty;        // 8 bytes
    // Range: Same as quantity
    // Total filled quantity * 10^8

    uint64_t lastQty;       // 8 bytes
    // Range: Same as quantity
    // Last fill quantity * 10^8

    uint64_t lastPx;        // 8 bytes
    // Range: Same as price
    // Last fill price * 10^8

} __attribute__((packed));

// Constants for field values
namespace FIX {
    namespace MsgType {
        static constexpr char 
            LOGON = 'A',
            LOGOUT = '5',
            NEW_ORDER = 'D',
            CANCEL = 'F',
            EXEC_REPORT = '8';
    }

    namespace Side {
        static constexpr char 
            BUY = '1',
            SELL = '2';
    }

    namespace OrdType {
        static constexpr char 
            MARKET = '1',
            LIMIT = '2';
    }

    namespace ExecType {
        static constexpr char 
            NEW = '0',
            CANCELED = '4',
            TRADE = 'F';
    }

    namespace OrdStatus {
        static constexpr char 
            NEW = '0',
            PARTIAL = '1',
            FILLED = '2',
            CANCELED = '4';
    }
}