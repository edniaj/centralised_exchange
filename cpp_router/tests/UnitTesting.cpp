#include <iostream>
#include "TestDatabaseManager.h"

int main() {
    std::cout << "Starting database tests..." << std::endl;
    
    try {
        TestDatabaseManager tester;
        tester.runAllTests();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with error: " << e.what() << std::endl;
        return 1;
    }
}