#ifndef REDIS_MANAGER_H
#define REDIS_MANAGER_H

#include <string>
#include <vector>
#include <sw/redis++/redis++.h>

class RedisManager {
private:
    sw::redis::Redis redis;

public:
    RedisManager(const std::string &uri);

    std::string ping();
    std::vector<std::string> getAllKeys();

    // Add any other public methods that are implemented in RedisManager.cpp

    // If there are any private methods or members, declare them here
};

#endif // REDIS_MANAGER_H
