FINANCIAL EXCHANGE SYSTEM - PROJECT PLAN
1. SYSTEM ARCHITECTURE RECAP
Copy[Client] → DNS LOADBALANCER → [FIX_GATEWAYS] → [MATCHING_ENGINE] → [Secondary Server]
Key Components:

FIX Gateway

Accepts FIX messages
Load balanced
Uses Ring Buffer
Converts to binary format
Journals messages


Matching Engine

Processes orders
Maintains order books
Batch updates to DB
Market data streaming


Database Layer

Already implemented (DatabaseManager)
Handles users, orders, trades, balances



2. REQUIRED FILES STRUCTURE
src/
├── network/
│   ├── SocketManager.h/.cpp      # TCP connection handling
│   ├── SessionManager.h/.cpp     # FIX session management
│   └── LoadBalancer.h/.cpp       # DNS load balancing
│
├── fix/
│   ├── FixMessage.h/.cpp         # FIX message structure
│   ├── FixParser.h/.cpp          # Message parsing
│   ├── FixGateway.h/.cpp         # Gateway implementation
│   └── FixConstants.h            # Constants
│
├── core/
│   ├── RingBuffer.h/.cpp         # Circular buffer
│   ├── Journal.h/.cpp            # Message journaling
│   └── BinaryEncoder.h/.cpp      # Message encoding
│
├── matching/
│   ├── MatchingEngine.h/.cpp     # Main engine
│   ├── OrderBook.h/.cpp          # Order book
│   ├── Order.h/.cpp              # Order structure
│   └── Trade.h/.cpp              # Trade structure
│
└── database/                     # Already implemented
    ├── DatabaseManager.h/.cpp
    └── DatabaseTypes.h