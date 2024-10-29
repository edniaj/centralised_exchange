#include <iostream>
#include "../include/TestDatabaseManager.h"
#include "../include/TestRedisManager.h"

int main() {
    try {
        // Database Tests
        std::cout << "\n=== Starting Database Tests ===\n" << std::endl;
        TestDatabaseManager dbTester;
        dbTester.runAllTests();

        // Redis Tests
        std::cout << "\n=== Starting Redis Tests ===\n" << std::endl;
        TestRedisManager redisTester;
        redisTester.runAllTests();

        std::cout << "\n=== All Tests Complete ===\n" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with error: " << e.what() << std::endl;
        return 1;
    }
}