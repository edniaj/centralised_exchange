#include <iostream>
#include <pqxx/pqxx>
#include <string>
#include <sw/redis++/redis++.h>

void test_redis()
{
    try
    {
        // Create a Redis object using a URI
        sw::redis::Redis redis("tcp://127.0.0.1:6379");

        // Test the connection with PING
        auto pong = redis.ping();
        std::cout << "Ping Response: " << pong << std::endl;

        // Get all keys
        std::vector<std::string> keys;
        redis.keys("*", std::back_inserter(keys));
        std::cout << "All keys in Redis:" << std::endl;
        for (const auto& key : keys)
        {
            std::cout << key << std::endl;
        }
    }
    catch (const sw::redis::Error &err)
    {
        std::cerr << "Redis Error: " << err.what() << std::endl;
    }
}

void test_db()
{
    try
    {
        // Connection string
        std::string connString = "dbname=docker host=localhost user=docker password=docker";

        // Establish a connection to the database
        pqxx::connection conn(connString);

        // Create a transaction
        pqxx::work txn(conn);

        // Execute the SQL query for 'admin'
        pqxx::result result_admin = txn.exec("SELECT * FROM users WHERE username = 'admin'");

        // Execute the SQL query for 'admin1'
        pqxx::result result_admin1 = txn.exec("SELECT * FROM users WHERE username = 'admin1'");

        // Print the results for 'admin'
        std::cout << "User data for 'admin':\n";

        for (const auto &row : result_admin)
        {
            for (const auto &field : row)
            {
                std::cout << field.c_str() << "\t";
            }
            std::cout << std::endl;
        }

        std::cout << "\n";

        // Print the results for 'admin1'
        if (result_admin1.empty())
        {
            std::cout << "No user found with username 'admin1'\n";
        }
        else
        {
            std::cout << "User data for 'admin1':\n";
            for (const auto &row : result_admin1)
            {
                for (const auto &field : row)
                {
                    std::cout << field.c_str() << "\t";
                }
                std::cout << std::endl;
            }
        }

        // Commit the transaction
        txn.commit();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}


int main()
{
    try
    {
        // test_db();
        test_redis();
        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error in main: " << e.what() << std::endl;
        return 1;
    }
}
