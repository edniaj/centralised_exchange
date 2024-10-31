'''
CREATE TABLE IF NOT EXISTS users (
    id SERIAL PRIMARY KEY,
    username VARCHAR(50) UNIQUE NOT NULL,
    password VARCHAR(100) NOT NULL,
    sendercompid INTEGER UNIQUE NOT NULL,
    created_at TIMESTAMP NOT NULL
        )
CREATE TABLE orders (
    id UUID PRIMARY KEY,
    user_id INTEGER REFERENCES users(id),
    symbol VARCHAR(10) NOT NULL,
    side VARCHAR(4) NOT NULL,  -- BUY/SELL
    price DECIMAL(20,8) NOT NULL,
    remaining_quantity DECIMAL(20,8) NOT NULL,
    filled_quantity DECIMAL(20,8) DEFAULT 0,
    created_at TIMESTAMP NOT NULL
);

CREATE TABLE trades (
    id UUID PRIMARY KEY,
    maker_order_id UUID REFERENCES orders(id),
    taker_user_id INTEGER REFERENCES users(id),
    quantity DECIMAL(20,8) NOT NULL,
    created_at TIMESTAMP NOT NULL
);

CREATE TABLE balances (
    user_id INTEGER REFERENCES users(id),
    asset VARCHAR(10) NOT NULL,
    amount DECIMAL(20,8) NOT NULL,
    PRIMARY KEY (user_id, asset)
);
''' 
import psycopg2
from datetime import datetime
import bcrypt

def create_tables(cur):
    # Users table (simplified sendercompid to integer)
    cur.execute("""
        CREATE TABLE IF NOT EXISTS users (
            id SERIAL PRIMARY KEY,
            username VARCHAR(50) UNIQUE NOT NULL,
            password VARCHAR(100) NOT NULL,
            sendercompid INTEGER UNIQUE NOT NULL,
            created_at TIMESTAMP NOT NULL
        )
    """)

    # Orders table
    cur.execute("""
        CREATE TABLE IF NOT EXISTS orders (
            id SERIAL PRIMARY KEY,
            user_id INTEGER REFERENCES users(id),
            symbol VARCHAR(10) NOT NULL,
            side VARCHAR(4) NOT NULL,
            price DECIMAL(20,8) NOT NULL,
            remaining_quantity DECIMAL(20,8) NOT NULL,
            filled_quantity DECIMAL(20,8) DEFAULT 0,
            created_at TIMESTAMP NOT NULL
        )
    """)

    # Trades table
    cur.execute("""
        CREATE TABLE IF NOT EXISTS trades (
            id SERIAL PRIMARY KEY,
            maker_order_id INTEGER REFERENCES orders(id),
            taker_user_id INTEGER REFERENCES users(id),
            quantity DECIMAL(20,8) NOT NULL,
            created_at TIMESTAMP NOT NULL
        )
    """)

    # Balances table
    cur.execute("""
        CREATE TABLE IF NOT EXISTS balances (
            user_id INTEGER REFERENCES users(id),
            asset VARCHAR(10) NOT NULL,
            amount DECIMAL(20,8) NOT NULL,
            PRIMARY KEY (user_id, asset)
        )
    """)

def create_admin_user(cur):
    # Generate admin password
    salt = bcrypt.gensalt()
    hashed_password = bcrypt.hashpw('password'.encode('utf-8'), salt)
    
    # Insert admin user with sendercompid as integer
    cur.execute("""
        INSERT INTO users (username, password, sendercompid, created_at)
        VALUES (%s, %s, %s, %s)
        ON CONFLICT (username) DO NOTHING
    """, ('admin', hashed_password.decode('utf-8'), 1, datetime.now()))

def main():
    try:
        # Connect to database
        conn = psycopg2.connect(
            dbname="docker",
            user="docker",
            password="docker",
            host="localhost"
        )
        
        cur = conn.cursor()
        create_tables(cur)
        create_admin_user(cur)
        conn.commit()
        print("Database setup completed successfully.")
        
    except Exception as e:
        print(f"Error: {e}")
        
    finally:
        if cur:
            cur.close()
        if conn:
            conn.close()

if __name__ == "__main__":
    main()