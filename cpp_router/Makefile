# Compiler to use
CC=g++

# Output target executable name
TARGET=server

# Source files
SOURCES=socket.cpp

# Libraries to link
LIBS=-lpqxx

# Rule to build the executable
$(TARGET): $(SOURCES)
	$(CC) $(SOURCES) -o $(TARGET) $(LIBS)

# Phony target for cleaning up
clean:
	rm -f $(TARGET)
