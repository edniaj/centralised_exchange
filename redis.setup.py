'''

1. Orders at Price Level (Sorted Set)
   Key: "{symbol}:{side}:{price}"
   Example: "AAPL:buy:150.00" or "AAPL:sell:151.50"
   Score: Timestamp (in milliseconds)
   Member: Order ID

2. Order Details (Hash)
   Key: "{symbol}:orders:{order_id}"
   Example: "AAPL:orders:1234567"
   Fields: (order_id, user_id, side, price, quantity, timestamp, status)

3. Aggregated Price Level Info (Hash)
   Key: "{symbol}:{side}:{price}:info"
   Example: "AAPL:buy:150.00:info"
   Fields: (total_quantity, order_count)

4. User Orders Index (Set)
   Key: "{symbol}:user:{user_id}:orders"
   Example: "AAPL:user:user123:orders"
   Members: Order IDs
   

'''

