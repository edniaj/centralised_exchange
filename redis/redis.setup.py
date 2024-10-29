'''
This files will create mock data inside redis

1. Order Details (Hash)
   Key: "{symbol}:orders:{order_id}"
   Example: "AAPL:orders:1234567"
   Fields: (order_id, user_id, symbol, side, price, quantity, timestamp)

2. User Orders Index (Set)
   Key: "user:{user_id}:orders"
   Example: "user:user123:orders"
   Members: Order IDs  

'''
import redis
import time
import random

class OrderGenerator:
    def __init__(self, symbol, db_num, base_price, spread=0.5):
        self.symbol = symbol
        self.r = redis.StrictRedis(host='localhost', port=6379, db=db_num)
        
        # Set price ranges (spread% above and below base price)
        self.min_buy_price = base_price * (1 - spread/100)
        self.max_buy_price = base_price
        self.min_sell_price = base_price
        self.max_sell_price = base_price * (1 + spread/100)
        
        # Set tick size based on price magnitude
        if base_price < 1000:  # AAPL
            self.tick_size = 0.01
        elif base_price < 5000:  # ETH
            self.tick_size = 0.1
        else:  # BTC
            self.tick_size = 1.0

    def generate_orders(self, num_users=1000, num_orders_per_user=5):
        # Clear existing data
        self.r.flushdb()
        
        # Generate user IDs
        user_ids = [f"user{i:04}" for i in range(1, num_users + 1)]
        order_id_counter = 1

        # Create single pipeline for all operations
        with self.r.pipeline() as pipe:
            pipe.multi()  # Start transaction

            for user_id in user_ids:
                for _ in range(num_orders_per_user):
                    side = random.choice(["buy", "sell"])
                    
                    if side == "buy":
                        price = round(random.uniform(self.min_buy_price, self.max_buy_price) / self.tick_size) * self.tick_size
                    else:
                        price = round(random.uniform(self.min_sell_price, self.max_sell_price) / self.tick_size) * self.tick_size

                    # Adjust quantity based on price
                    if self.symbol == "BTC":
                        quantity = round(random.uniform(0.1, 2), 3)
                    elif self.symbol == "ETH":
                        quantity = round(random.uniform(1, 10), 2)
                    else:
                        quantity = random.randint(1, 100)

                    timestamp = int(time.time() * 1000)
                    order_id = f"{order_id_counter:07}"  # Just numbers, no symbol prefix

                    # Add commands to pipeline
                    pipe.hset(f"orders:{order_id}", mapping={
                        "user_id": user_id,
                        "symbol": self.symbol,
                        "side": side,
                        "price": f"{price:.2f}",
                        "quantity": quantity,
                        "timestamp": timestamp,
                    })
                    pipe.sadd(f"user:{user_id}:orders", order_id)

                    order_id_counter += 1

            # Execute all commands in single transaction
            try:
                pipe.execute()
            except Exception as e:
                print(f"Failed to execute pipeline for {self.symbol}: {e}")
                return False

        return True

def main():
    symbols_config = [
        {"symbol": "AAPL", "db": 0, "price": 233.0},
        {"symbol": "BTC", "db": 1, "price": 71100.0},
        {"symbol": "ETH", "db": 2, "price": 3610.0}
    ]

    for config in symbols_config:
        print(f"\nGenerating orders for {config['symbol']} in database {config['db']}...")
        generator = OrderGenerator(
            symbol=config['symbol'],
            db_num=config['db'],
            base_price=config['price']
        )
        success = generator.generate_orders(num_users=200, num_orders_per_user=4)
        if success:
            print(f"Orders generated successfully for {config['symbol']}")
        else:
            print(f"Failed to generate orders for {config['symbol']}")

if __name__ == "__main__":
    main()