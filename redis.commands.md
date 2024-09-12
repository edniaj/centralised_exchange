## Common short-forms used in commands
    H - Hash
    S - Set
    Z - Sorted Set

    REV - Reverse
    0 -1 - First index item to the last index item

## Destroying Redis Cache
FLUSHDB SYNC  
    - Synchronous deletion of current database

FLUSHALL SYNC  
    - Synchronous deletion of all databases

## Common Commands to Navigate
SELECT 'index number of the database'  
    - Select database by index

KEYS *  
    - Show all keys in the selected database

## Common Commands with Sorted Sets (Z Commands)
ZADD 'sorted_set' 'score' 'member'  
    - Add members to a sorted set

ZRANGE 'sorted_set' 0 -1  
    - Get all members in a sorted set

ZRANGE 'sorted_set' 0 -1 WITHSCORES  
    - Get all members with their scores

ZREM 'sorted_set' 'member'  
    - Remove a member from a sorted set

ZRANK 'sorted_set' 'member'  
    - Get the rank of a member (lowest to highest)

ZREVRANK 'sorted_set' 'member'  
    - Get the rank of a member (highest to lowest)

ZRANGEBYSCORE 'sorted_set' 'min_score' 'max_score'  
    - Get members by score range

ZCOUNT 'sorted_set' 'min_score' 'max_score'  
    - Count members by score range

ZREMRANGEBYSCORE 'sorted_set' 'min_score' 'max_score'  
    - Remove members by score range

ZSCAN 'sorted_set' 0  
    - Incrementally iterate over sorted set

## Common Commands with Sets
SADD 'set' 'member1' 'member2'  
    - Add one or more members to a set

SMEMBERS 'set'  
    - Get all members in the set

SREM 'set' 'member'  
    - Remove a member from a set

SISMEMBER 'set' 'member'  
    - Check if a member exists in the set (returns 1 if true, 0 if false)

SCARD 'set'  
    - Get the number of members in the set

SDIFF 'set1' 'set2'  
    - Return the difference between two sets

SINTER 'set1' 'set2'  
    - Return the intersection of two sets

SUNION 'set1' 'set2'  
    - Return the union of two sets

## Common Commands with Hashes
HSET 'hash' 'field' 'value'  
    - Set the value of a field in a hash

HGET 'hash' 'field'  
    - Get the value of a field in a hash

HGETALL 'hash'  
    - Get all fields and values in a hash

HDEL 'hash' 'field'  
    - Delete a field from a hash

HEXISTS 'hash' 'field'  
    - Check if a field exists in a hash (returns 1 if true, 0 if false)

HLEN 'hash'  
    - Get the number of fields in a hash

HINCRBY 'hash' 'field' 'increment'  
    - Increment the value of a field by a number

HSCAN 'hash' 0  
    - Incrementally iterate over fields in a hash
