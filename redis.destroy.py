import redis

'''
This file will flush all Redis databases synchronously.
'''

# Connect to Redis
r = redis.StrictRedis(host='localhost', port=6379, db=0)

# Flush all Redis databases synchronously
try:
    r.flushall(asynchronous=False)  # `False` ensures synchronous flush
    print("All Redis databases have been flushed synchronously.")
except redis.RedisError as e:
    print(f"An error occurred: {e}")
