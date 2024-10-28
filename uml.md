# UML Class Diagram

## Class Composition

## Class Descriptions and Methods

### Orderbook
- **Purpose**: Manages the order book for a specific symbol.
- **Methods**:
  - `addOrder(Order): void`
  - `removeOrder(string): void`
  - `modifyOrder(string, int): void`
  - `getOrdersAtPrice(string, double): vector<Order>`
  - `getPriceLevelInfo(string, double): PriceLevelInfo`
  - `getUserOrders(string): vector<string>`

### Order
- **Purpose**: Represents an individual order in the system.
- **Attributes**: order_id, user_id, side, price, quantity, timestamp, status

### PriceLevelInfo
- **Purpose**: Stores aggregated information for a specific price level.
- **Attributes**: total_quantity, order_count

### RedisManager
- **Purpose**: Handles interactions with Redis for caching and real-time data.
- **Methods**:
  - `connect(): void`
  - `disconnect(): void`
  - `setOrder(Order): void`
  - `getOrder(string): Order`
  - `updatePriceLevelInfo(string, double, PriceLevelInfo): void`

### DatabaseManager
- **Purpose**: Manages persistent storage and retrieval of data.
- **Methods**:
  - `connect(): void`
  - `disconnect(): void`
  - `executeQuery(string): ResultSet`
  - `insertOrder(Order): void`
  - `updateOrder(Order): void`
  - `deleteOrder(string): void`

### SocketManager
- **Purpose**: Handles WebSocket connections for real-time updates.
- **Methods**:
  - `start(): void`
  - `stop(): void`
  - `broadcastUpdate(string): void`
  - `sendToClient(string, string): void`

### SessionManager
- **Purpose**: Manages user sessions and authentication.
- **Methods**:
  - `createSession(string): string`
  - `validateSession(string): bool`
  - `endSession(string): void`

### SymbolRouter
- **Purpose**: Routes orders and queries to the appropriate Orderbook based on the symbol.
- **Methods**:
  - `addOrder(string, Order): void`
  - `removeOrder(string, string): void`
  - `modifyOrder(string, string, int): void`
  - `getOrderbook(string): Orderbook&`
