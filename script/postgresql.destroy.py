import psycopg2 #PostgreSQL database adapter 
from psycopg2 import sql

# Connect to the database
conn = psycopg2.connect(
    dbname="docker",
    user="docker",
    password="docker",
    host="localhost"
)

# Create a cursor
cur = conn.cursor()

# Get a list of all tables in the database
cur.execute("""
    SELECT table_name FROM information_schema.tables
    WHERE table_schema = 'public'
""")
tables = cur.fetchall()

# Drop all tables
for table in tables:
    table_name = table[0]
    cur.execute(sql.SQL("DROP TABLE IF EXISTS {} CASCADE").format(sql.Identifier(table_name)))
    print(f"Dropped table: {table_name}")

# Commit the changes
conn.commit()

# Close the cursor and connection
cur.close()
conn.close()

print("All tables have been destroyed successfully.")