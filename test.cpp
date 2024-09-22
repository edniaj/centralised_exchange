#include <iostream>
#include <pqxx/pqxx>
#include <string>

int main()
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
            // Commit the transaction
            txn.commit();
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
