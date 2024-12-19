// FixGateway.h
#pragma once  // Prevents multiple inclusions of this header

#include <unordered_map>
#include <string>
#include <memory>
#include "SessionManager.h"  // You'll create this later
#include "MatchingEngine.h"    // You'll create this later
#include "FixMessage.h"
#include "DatabaseManager.h"
#include "MatchingEngine.h"
#include "SocketManager.h"
#include "BinaryEncoder.h"
class FIXGateway {
private:
    std::string gateway_id; // This is the ID of the FIX Gateway
    
    SessionManager sessionManager; // We will use this to manage the sessions and make sure the client is authenticated
    DatabaseManager databaseManager; // We will use this to manage the database
    MatchingEngine matchingEngine; // We will use this to route the orders to the correct Matching Engine Instance
    BinaryEncoder binaryEncoder;
    
    // Private methods
    bool validateMessage(const FIXMessage& msg); // Checksum of FixMessage and TargetCompID must match
    void logMessage(const std::string& msg); // Log the message to the console

public:
    // Constructor
    FIXGateway(const std::string& id, DatabaseManager& db, SymbolRouter& router);
    
    // Public interface
    bool handleNewConnection(int client_fd);
    void handleOrder(FixMessage& msg); // Route the order to the correct Matching Engine Instance
    bool processMessage(const FIXMessage& msg);
    void disconnect(int client_fd); // We will close socket here
    
    // Getters
    std::string getGatewayId() const { return gateway_id; }

    
};