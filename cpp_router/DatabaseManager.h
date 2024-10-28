#ifndef DATABASE_MANAGER_H
#define DATABASE_MANAGER_H

#include <string>
#include <pqxx/pqxx>

class DatabaseManager {
private:
    pqxx::connection conn;

public:
    DatabaseManager(const std::string &connString);

    bool verifyUser(const std::string &username, const std::string &password);
    std::string getUserSenderCompId(const std::string &username);

    // Add any other public methods that are implemented in the DatabaseManager class

    // If there are any private methods or members, declare them here
};

#endif // DATABASE_MANAGER_H
