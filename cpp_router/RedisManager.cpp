#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <sw/redis++/redis++.h>

class RedisManager
{
private:
    sw::redis::Redis redis;

public:
    RedisManager(const std::string &uri) : redis(uri)
    {
        auto keys = getAllKeys();
        if (keys.empty())
        {
            throw std::runtime_error("Redis database is empty. At least one key should exist.");
        }
        else
        {
            std::cout << "Redis contains " << keys.size() << " key(s).\n=============================\n"
                      << std::endl;
        }
    }

    std::string ping()
    {
        return redis.ping();
    }

    std::vector<std::string> getAllKeys()
    {
        std::vector<std::string> keys;
        redis.keys("*", std::back_inserter(keys));
        return keys;
    }
};