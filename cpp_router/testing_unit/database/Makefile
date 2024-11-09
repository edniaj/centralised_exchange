# Compiler settings
CC=g++ -std=c++17

# Directories
INCLUDE_DIR=../../include/database
SOURCE_DIR=../../source/database
TEST_DIR=.

# Output executable
TARGET=unit_tests

# Source files
TEST_SOURCES=$(TEST_DIR)/UnitTesting.cpp \
             $(TEST_DIR)/TestDatabaseManager.cpp \
             $(TEST_DIR)/TestRedisManager.cpp

SOURCE_FILES=$(SOURCE_DIR)/DatabaseManager.cpp \
            $(SOURCE_DIR)/RedisManager.cpp

# Include paths
INCLUDES=-I$(INCLUDE_DIR) -I/usr/include/postgresql

# Libraries to link
LIBS=-lpqxx -lpq -lredis++ -lhiredis

# Compilation flags
CFLAGS=-Wall -Wextra

# Rule to build the executable
$(TARGET): $(TEST_SOURCES) $(SOURCE_FILES)
	$(CC) $(CFLAGS) $(INCLUDES) $(TEST_SOURCES) $(SOURCE_FILES) -o $(TARGET) $(LIBS)

# Clean rule
clean:
	rm -f $(TARGET)

# Run rule
run: $(TARGET)
	./$(TARGET)

# Debug build
debug: CFLAGS += -g -DDEBUG
debug: $(TARGET)
