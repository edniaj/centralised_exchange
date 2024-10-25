import psycopg2 #PostgreSQL database adapter 
from psycopg2 import sql
from datetime import datetime
import bcrypt
import random
import string

# Connect to the database
conn = psycopg2.connect(
    dbname="docker",
    user="docker",
    password="docker",
    host="localhost"
)

# Create a cursor
cur = conn.cursor()

'''Create the users table
We dont need a separate column to store the salt since its appended to the hash (bcrypt)
Added sendercompid field for FIX protocol
'''
cur.execute("""
    CREATE TABLE IF NOT EXISTS users (
        id SERIAL PRIMARY KEY,
        username VARCHAR(50) UNIQUE NOT NULL,
        password VARCHAR(100) NOT NULL,
        sendercompid VARCHAR(20) UNIQUE NOT NULL,
        created_at TIMESTAMP NOT NULL
    )
""") 

# Generate a salt and hash the password
salt = bcrypt.gensalt()
hashed_password = bcrypt.hashpw('password'.encode('utf-8'), salt)

# Generate a unique sendercompid
def generate_sendercompid(username):
    prefix = 'CEX'
    random_suffix = ''.join(random.choices(string.digits, k=5))
    return f"{prefix}_{username.upper()}_{random_suffix}"

# Insert mock admin user
admin_sendercompid = generate_sendercompid('admin')
admin_user = ('admin', hashed_password.decode('utf-8'), admin_sendercompid, datetime.now())

cur.execute("""
    INSERT INTO users (username, password, sendercompid, created_at)
    VALUES (%s, %s, %s, %s)
    ON CONFLICT (username) DO NOTHING
""", admin_user)

# Commit the changes
conn.commit()

# Close the cursor and connection
cur.close()
conn.close()

print("Database setup completed successfully.")
