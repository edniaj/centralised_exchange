class DatabaseManager
{
private:
    pqxx::connection conn;

public:
    DatabaseManager(const std::string &connString) : conn(connString)
    {
        try
        {
            pqxx::work txn(conn);
            pqxx::result result = txn.exec("SELECT COUNT(*) FROM users");
            txn.commit();

            if (!result.empty())
            {
                int userCount = result[0][0].as<int>();
                std::cout << "\n=============================\nDatabase contains " << userCount << " user(s)." << std::endl;
            }
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error counting users: " << e.what() << std::endl;
        }
    }

    bool verifyUser(const std::string &username, const std::string &password)
    {
        try
        {
            pqxx::work txn(conn);
            pqxx::result result = txn.exec(
                "SELECT * FROM users WHERE username = " + txn.quote(username));

            if (result.empty())
            {
                return false; // User not found
            }

            // Here you would typically check the password
            // For this example, we're just checking if the user exists
            // In a real application, you'd need to hash the password and compare it

            txn.commit();
            return true;
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error verifying user: " << e.what() << std::endl;
            return false;
        }
    }

    std::string getUserSenderCompId(const std::string &username)
    {
        try
        {
            pqxx::work txn(conn);
            pqxx::result result = txn.exec(
                "SELECT sendercompid FROM users WHERE username = " + txn.quote(username));

            if (result.empty())
            {
                return ""; // User not found, return empty string
            }

            txn.commit();
            return result[0][0].as<std::string>();
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error fetching user sendercompid: " << e.what() << std::endl;
            return ""; // Return empty string on error
        }
    }

    // You can add more methods here for other database operations
};