# Compiler to use , must be c++17 and above for redis++
CC=g++ -std=c++17

# Output target executable name
TARGET=server

# Source files
SOURCES=socket.cpp

# Libraries to link
LIBS=-lpqxx -pthread -lredis++ -lhiredis

# Rule to build the executable
$(TARGET): $(SOURCES)
	$(CC) $(SOURCES) -o $(TARGET) $(LIBS)

# Phony target for cleaning up
clean:
	rm -f $(TARGET) $(TARGET).exe
