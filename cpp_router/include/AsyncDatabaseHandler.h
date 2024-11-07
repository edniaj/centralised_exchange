// AsyncDatabaseHandler.h

/*
Problem:
1. Matching Engine needs to write to DB (orders/trades)
2. Direct DB operations slow down matching
3. Pipe system can fail if executePipe() runs while matching engine tries to add operations
4. Need thread-safe way to batch DB operations without blocking matching

Solution (AsyncDatabaseHandler):
1. Matching Engine just queues operations (fast, non-blocking)
2. Background thread manages DB pipe lifecycle:
   - Start pipe
   - Flush queued operations
   - Execute pipe
   - Repeat
3. Thread-safe queue ensures no operations are lost

Result: Fast matching engine with a queue as BUFFER for the pipe.

Matching Engine                         AsyncDatabaseHandler
    [Match Orders] ----queue op----->   [Collect Operations]
    [Continue Matching]                 [Batch Write to DB every X ms]

    ≽^•⩊•^≼
     
*/
    
// AsyncDatabaseHandler.h
#pragma once

#include <thread>
#include <atomic>
#include "DatabaseManager.h"
#include "LockFreeQueue.h" // We'll create this

enum class DBOperationType {
    CREATE_ORDER,
    UPDATE_ORDER_QUANTITY,
    UPDATE_ORDER_FILLED,
    DELETE_ORDER,
    CREATE_TRADE
};

struct DBOperation {
    DBOperationType type;
    // Parameters for each operation type
    union {
        struct { int userId; std::string symbol; std::string side; double price; double qty; } create;
        struct { int orderId; double qty; } update;
        struct { int makerId; int takerId; double qty; } trade;
    } params;
};

class AsyncDatabaseHandler {
private:
    DatabaseManager& dbManager;
    LockFreeQueue<DBOperation> operationQueue;
    std::thread flushThread;
    std::atomic<bool> running{false};
    
    void flushPeriodically();

public:
    AsyncDatabaseHandler(DatabaseManager& db);
    ~AsyncDatabaseHandler();

    void start();
    void stop();
    void queueOperation(DBOperation op); // Non-blocking
};