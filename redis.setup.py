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
import random

# Connect to Redis
r = redis.StrictRedis(host='localhost', port=6379, db=0)

# Sample data
symbol = "AAPL"
side_buy = "buy"
side_sell = "sell"

# Price range and tick size
min_price = 145.50
max_price = 155.50
tick_size = 0.1

# Define the number of users and orders
num_users = 100
num_orders_per_user = 50

# Generate user and order IDs
user_ids = [f"user{i:04}" for i in range(1, num_users + 1)]

def create_order(user_id, order_id, side, price, quantity, timestamp):
    """
    Create an order and submit it to Redis within a transaction.
    """
    try:
        with r.pipeline() as pipe:
            # Start a Redis transaction (MULTI)
            pipe.multi()

            # 1. Orders at Price Level (Sorted Set)
            pipe.zadd(f"{symbol}:{side}:{price}", {order_id: timestamp})

            # 2. Order Details (Hash)
            pipe.hset(f"{symbol}:orders:{order_id}", mapping={
                "order_id": order_id,
                "user_id": user_id,
                "side": side,
                "price": price,
                "quantity": quantity,
                "timestamp": timestamp,
                "status": "open"
            })

            # 3. Aggregated Price Level Info (Hash)
            pipe.hincrby(f"{symbol}:{side}:{price}:info", "total_quantity", quantity)
            pipe.hincrby(f"{symbol}:{side}:{price}:info", "order_count", 1)

            # 4. User Orders Index (Set)
            pipe.sadd(f"{symbol}:user:{user_id}:orders", order_id)

            # Execute all commands atomically (EXEC)
            pipe.execute()

        print(f"Order {order_id} for user {user_id} at price {price} submitted successfully.")

    except Exception as e:
        print(f"Failed to submit order {order_id} for user {user_id}: {e}")

def generate_orders():
    """
    Generate and submit multiple orders for each user.
    """
    order_id_counter = 1

    for user_id in user_ids:
        for _ in range(num_orders_per_user):
            # Random side: buy or sell
            side = random.choice([side_buy, side_sell])

            # Random price within the given range with the defined tick size
            price = round(random.uniform(min_price, max_price) // tick_size * tick_size, 2)

            # Random quantity between 1 and 100
            quantity = random.randint(1, 100)

            # Current timestamp in milliseconds
            timestamp = int(time.time() * 1000)

            # Generate order ID
            order_id = f"{order_id_counter:07}"

            # Create the order
            create_order(user_id, order_id, side, price, quantity, timestamp)

            # Increment the order ID counter
            order_id_counter += 1

if __name__ == "__main__":
    generate_orders()
