# Compiler to use
CC=g++ -std=c++17

# Output target executable name
TARGET=test

# Source files
SOURCES=test.cpp

# Libraries to link
LIBS = -lredis++ -lhiredis -lpqxx

# Rule to build the executable
$(TARGET): $(SOURCES)
	$(CC) $(SOURCES) -o $(TARGET) $(LIBS)

# Phony target for cleaning up
clean:
	rm -f $(TARGET)
