// SymbolRouter.h
#pragma once

#include <string>
#include <unordered_map>

class SymbolRouter
{
private:
    // Maps symbols to their designated matching engine IDs
    std::unordered_map<std::string, std::string> symbolToEngine; // symbol -> engineId

public:
    SymbolRouter();

    // Core routing functionality
    std::string getEngineForSymbol(const std::string &symbol) const;

    // Symbol assignment management
    void addRoute(const std::string &symbol, const std::string &engineId);
    void removeRoute(const std::string &symbol);
    bool hasRoute(const std::string &symbol) const;

    // Load/Update routing table
    void loadRoutingTable(); // Load initial routing configuration
    void updateRoutingTable(const std::string &symbol, const std::string &engineId);
};