// DatabaseManager.h
#ifndef DATABASE_MANAGER_H
#define DATABASE_MANAGER_H

#include <string>
#include <vector>
#include <optional>
#include <pqxx/pqxx>

// Forward declaration of User struct
struct User {
    int id;
    std::string username;
    std::string password;
    std::string senderCompId;
    std::string createdAt;
};

class DatabaseManager {
private:
    pqxx::connection conn;

public:
    // Constructor
    DatabaseManager(const std::string& connString);

    // Authentication
    bool verifyUser(const std::string& username, const std::string& password);

    // CRUD Operations
    bool createUser(const std::string& username, const std::string& password, 
                   const std::string& senderCompId);
    
    std::vector<User> readAllUsers();
    std::optional<User> readUserByUsername(const std::string& username);
    std::optional<std::string> readSenderCompIdByUsername(const std::string& username);
    
    bool updateUser(int userId, const std::string& newPassword, 
                   const std::string& newSenderCompId);
    
    bool deleteUser(int userId);

    // Connection status
    bool isConnected() const { return conn.is_open(); }
};

#endif // DATABASE_MANAGER_H
