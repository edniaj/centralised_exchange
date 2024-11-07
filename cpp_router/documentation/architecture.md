# Architecture
## LATEST ARCHITECTURE 8 NOV 2024

DNS LOADBALANCER -- REDIRECTS --> [FIX_GATEWAY_1, FIX_GATEWAY_2 ... FIX_GATEWAY_N]

### FIX GATEWAY  
--> PRODUCER WRITES ORIGINAL FIX MESSAGE INTO RING BUFFER  
--> RING_BUFFER { 
    seq_1 :JOURNAL ORIGINAL FIX MESSAGE,
    seq_2: BINARY ENCODE FIX_MESSAGE, 
    seq_3: FORWARD ENCODED MESSAGE TO 2 routes - 
        first_route = matching engine, 
        second_route= secdonary_server(replication)}
          explaination: secondary_server will be ready to take over if original ME or all FIX gateway are dead (e.g.DDOS, bla bla)

Secondary server might not have exact same state. Race condition between different gateways - solve this

### MATCHING ENGINE
--> PRODUCER WRITE BINARY ENCODED MESSAGE INTO RING BUFFER
RINGBUFFER {seq_1: journal binary_encoded message
seq_2: match order and execute db pipe command if needed 
seq_3: maybe stream new change to orderbook or something ? unsure - resolve this tmr 
}
Receives binary encoded message --> MATCHES ORDERS -> ADD TO DATABSE OPERATION - UPDATE DB BY BATCHES
-- MARKET DATA RING BUFFER --> RINGBUFFER{  
  
                              seq1: write to publishing_data_array, 
                              seq2: write data to market data buffer for streaming (figure out how to implement later)
                              }                  

we need to journal the binary encoded messages so that it can be replayed in SEQUENCE to reconstruct database !!
1. FIX Gateway RingBuffer:
[Receive FIX] -> RingBuffer {
    |-> Journal Handler (FIX audit)
    |-> Encode Handler (binary)
    |-> Replicate Handler (secondary)
    |-> Forward Handler (to ME)
}

2. Matching Engine:
[Receive Binary] {
    - Match orders (single thread)
    - Update local state
    - Pipe DB operations (batch processing)
    - Push to Market Data Buffer
}

3. Market Data RingBuffer:
[Market Updates] -> RingBuffer {
    |-> Publisher Handler
    |-> Feed Handler
}

FIX: 
AVOID USING QUEUES - LARGE OVERHEAD DUE TO CONTENTION AND BLA BLA
IMPLEMENT DISRUPTOR PATTERN CIRCULAR BUFFER

CENTRALISED MATCHING ENGINE - RESOLVES PROBLEM ABOUT SYNCING BALANCE
LOWER UPDATING DATABASE OVERHEAD COST SINCE WE INTRODUCED FAULT TOLERANCE METHOD (JOURNALLING)

Problem: Secondary server might not have exact same state. Race condition between different gateways

I will describe the architecture of the system in a way that is easy to understand.
## CORRECT:

### 1. Connection Layer:
[FIX Clients] → [DNS/Load Balancer] → [Multiple FIX Gateways]
                                      (Each gateway handles max N connections)
#### Explaination

DNS Load Balancer will issue Fix Gateway IP address to the FIX Clients.
FIX Clients will send trade orders to Fix Gateway which will redirect the orders to the correct Symbol Router.
Fix gateway (brokers) will also check balance before sending trade orders, if balance low then dont send to matching engine


This architecture will allow us to horizontally scale the FIX Gateways so that MORE USERS can trade. 

Pain points: We do not want users to directly connect to the Matching Engine because the matching engine wants to resolve trades locally thus it is not ideal to horizontally scale the Matching Engine, it will make more sense to vertically scale the Matching Engine.


### 2. Order Processing:
[FIX Gateway] → [FIX GATEWAY :: Symbol Router] → [Matching Engine Instance]
                                  (Each symbol assigned to ONE specific instance)
#### Explaination
FIX Gateway will send the orders to the Symbol Router.
Symbol Router will then direct the orders to the correct Matching Engine Instance.

### 3. Persistence:
[Matching Engine] → POSTGRESQL

#### Explaination
Trades should be resolved locally since we want to implement some algorithm for the matching engine. 

We do not want to resolve the trade directly on redis because theres too much overhead + it will be very expensive to do it on AWS.



## INCORRECT (What we want to avoid):
[Client A] AAPL Order ----→ [Matching Engine 1] (Processing AAPL)
[ClientX B] AAPL Order ----→ [Matching Engine 2] (Also Processing AAPL) ❌ Race condition because Matching Engine doesn't know about other MEs and the trades are resolved locally.

# Database Architecture
# Trading System Database Architecture

## Core Components

### 1. User Management
- `users` table is the foundation
- Each user has:
  * Unique integer ID (auto-incrementing)
  * Unique username
  * Hashed password
  * Integer sendercompid for FIX protocol 
  * Account creation timestamp

### 2. Order Management
- `orders` table tracks all limit orders
- Each order links to a user (user_id foreign key)
- Tracks:
  * Order state through remaining_quantity
  * Execution progress through filled_quantity
  * Price and symbol information
  * Creation time for order priority

### 3. Trade Execution
- `trades` table records all matches
- Each trade has:
  * Maker (limit order) via maker_order_id
  * Taker (market participant) via taker_user_id
  * Executed quantity
  * Timestamp of execution

### 4. Balance Tracking
- `balances` table maintains user assets
- Composite key of user_id and asset ensures one balance per asset per user
- Tracks exact amounts with high precision decimals

## Relationships Flow

```plaintext
Users
  ↓
  ├──→ Orders (places orders)
  │     ↓
  │     └──→ Trades (order gets filled)
  │           ↑
  └───────────┘ (user takes trades)
  │
  └──→ Balances (maintains asset amounts)

### Serialization / Deserialization:
THERE IS NO NEED FOR ULTRA OPTIMIZATION HERE. WE WANT THIS TO BE READABLE AND MAINTAINABLE.
AS LONG AS WE CAN FINISH WRITING AND EXECUTE THE PIPELINE IN A REASONABLE TIME.


### Scaling more token options
Redis by default have 16 databases that can be edited in the edis.conf:


