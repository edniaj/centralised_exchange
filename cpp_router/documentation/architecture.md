# Architecture
I will describe the architecture of the system in a way that is easy to understand.
## CORRECT:

### 1. Connection Layer:
[FIX Clients] → [DNS/Load Balancer] → [Multiple FIX Gateways]
                                      (Each gateway handles max N connections)
#### Explaination
DNS Load Balancer will issue Fix Gateway IP address to the FIX Clients.
FIX Clients will send trade orders to Fix Gateway which will redirect the orders to the correct Symbol Router.

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


