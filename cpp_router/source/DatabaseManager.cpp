#include "../include/DatabaseManager.h"
#include <iostream>
#include <optional>


// Just implement the methods
DatabaseManager::DatabaseManager(const std::string &connString) : conn(connString)
{
    try
    {
        pqxx::work txn(conn); // Same as writing pqxx:work txn = pqxx::work(conn);
        pqxx::result result = txn.exec("SELECT COUNT(*) FROM users");
        txn.commit(); // Releases Locks on the rows that were read.

        if (!result.empty())
        {
            int userCount = result[0][0].as<int>(); // as<Type>() is a pqxx template method that converts the value to the specified type.
            std::cout << "\n=============================\nDatabase contains " << userCount << " user(s)." << std::endl;
        }
        else
        {
            std::cout << "No user found" << std::endl;
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error counting users: " << e.what() << std::endl;
    }
}

bool DatabaseManager::verifyUser(const std::string &username, const std::string &password)
{
    try
    {
        pqxx::work txn(conn);
        pqxx::result result = txn.exec_params(
            "SELECT password FROM users WHERE username = $1",
            username);

        if (result.empty())
        {
            return false; // User not found
        }

        std::string storedPassword = result[0][0].as<std::string>();
        txn.commit();

        // Compare the stored password with the provided password
        return (storedPassword == password);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error verifying user: " << e.what() << std::endl;
        return false;
    }
}

// CRUD Operations for USER table //
// CREATE - Insert a new user
bool DatabaseManager::createUser(const std::string &username, const std::string &password,
                                  const std::string &senderCompId)
{
    try
    {
        pqxx::work txn(conn);

        pqxx::result result = txn.exec_params(
            "INSERT INTO users (username, password, sendercompid, created_at) "
            "VALUES ($1, $2, $3, CURRENT_TIMESTAMP) RETURNING id",
            username, password, senderCompId);

        txn.commit();
        return !result.empty();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error creating user: " << e.what() << std::endl;
        return false;
    }
}

// READ - 1. Read all users | 2. Read user by username | 3. Read user by senderCompId

std::vector<User> DatabaseManager::readAllUsers()
{
    std::vector<User> users;
    try
    {
        pqxx::work txn(conn);

        pqxx::result result = txn.exec(
            "SELECT id, username, password, sendercompid, created_at "
            "FROM users ORDER BY id");

        txn.commit();

        for (const auto &row : result)
        {
            users.push_back(User{
                row[0].as<int>(),         // id
                row[1].as<std::string>(), // username
                row[2].as<std::string>(), // password
                row[3].as<std::string>(), // sendercompid
                row[4].as<std::string>()  // created_at
            });
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error fetching all users: " << e.what() << std::endl;
    }
    return users;
}
// READ - Get user by username
std::optional<User> DatabaseManager::readUserByUsername(const std::string &username)
{
    try
    {
        pqxx::work txn(conn);

        pqxx::result result = txn.exec_params(
            "SELECT id, username, password, sendercompid, created_at "
            "FROM users WHERE username = $1",
            username);

        txn.commit();

        if (result.empty())
        {
            return std::nullopt;
        }

        User user{
            result[0][0].as<int>(),         // id
            result[0][1].as<std::string>(), // username
            result[0][2].as<std::string>(), // password
            result[0][3].as<std::string>(), // sendercompid
            result[0][4].as<std::string>()  // created_at
        };

        return user;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error fetching user: " << e.what() << std::endl;
        return std::nullopt;
    }
}

std::optional<std::string> DatabaseManager::readSenderCompIdByUsername(const std::string &username)
{
    try
    {
        pqxx::work txn(conn);
        pqxx::result result = txn.exec(
            "SELECT sendercompid FROM users WHERE username = " + txn.quote(username));

        if (result.empty())
        {
            return std::nullopt; // User not found, return nullopt
        }

        txn.commit();
        return result[0][0].as<std::string>();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error fetching user sendercompid: " << e.what() << std::endl;
        return std::nullopt; // Return nullopt on error
    }
}
// UPDATE - Update user details
bool DatabaseManager::updateUser(int userId, const std::string &newPassword,
                                   const std::string &newSenderCompId)
{
    try
    {
        pqxx::work txn(conn);

        pqxx::result result = txn.exec_params(
            "UPDATE users "
            "SET password = $1, sendercompid = $2 "
            "WHERE id = $3 "
            "RETURNING id",
            newPassword, newSenderCompId, userId);

        txn.commit();
        return !result.empty();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error updating user: " << e.what() << std::endl;
        return false;
    }
}
// DELETE - Delete a user
bool DatabaseManager::deleteUser(int userId)
{
    try
    {
        pqxx::work txn(conn);

        pqxx::result result = txn.exec_params(
            "DELETE FROM users WHERE id = $1 RETURNING id",
            userId);

        txn.commit();
        return !result.empty();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error deleting user: " << e.what() << std::endl;
        return false;
    }
}
