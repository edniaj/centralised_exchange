# Compiler settings
CC=g++ -std=c++17

# Directories
INCLUDE_DIR=../include
SOURCE_DIR=../source

# Output executable
TARGET=test_database

# Source files
TEST_SOURCE=TestDatabaseManager.cpp
DB_SOURCE=$(SOURCE_DIR)/DatabaseManager.cpp

# Include paths
INCLUDES=-I$(INCLUDE_DIR)

# Libraries to link
LIBS=-lpqxx

# Compilation flags
CFLAGS=-Wall -Wextra

# Rule to build the executable
$(TARGET): $(TEST_SOURCE) $(DB_SOURCE)
	$(CC) $(CFLAGS) $(INCLUDES) $(TEST_SOURCE) $(DB_SOURCE) -o $(TARGET) $(LIBS)

# Clean rule
clean:
	rm -f $(TARGET)

# Run rule
run: $(TARGET)
	./$(TARGET)