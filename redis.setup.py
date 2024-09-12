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

import redis
import time

# Connect to Redis
r = redis.StrictRedis(host='localhost', port=6379, db=0)

# Sample data
symbol = "AAPL"
side_buy = "buy"
side_sell = "s11ell"
price = 150.00
order_id = "1234456337"
user_id = "user133234"
quantity = 10
timestamp = int(time.time() * 1000)  # Current time in milliseconds
status = "open"

# Start a Redis transaction
with r.pipeline() as pipe:
    # 1. Orders at Price Level (Sorted Set)
    pipe.zadd(f"{symbol}:{side_buy}:{price}", {order_id: timestamp})

    # 2. Order Details (Hash)
    pipe.hset(f"{symbol}:orders:{order_id}", mapping={
        "order_id": order_id,
        "user_id": user_id,
        "side": side_buy,
        "price": price,
        "quantity": quantity,
        "timestamp": timestamp,
        "status": status
    })

    # 3. Aggregated Price Level Info (Hash)
    pipe.hincrby(f"{symbol}:{side_buy}:{price}:info", "total_quantity", quantity)
    pipe.hincrby(f"{symbol}:{side_buy}:{price}:info", "order_count", 1)

    # 4. User Orders Index (Set)
    pipe.sadd(f"{symbol}:user:{user_id}:orders", order_id)

    # Execute all commands atomically
    pipe.execute()
