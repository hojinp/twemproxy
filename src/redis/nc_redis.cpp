#include "nc_redis.h"

#include <sw/redis++/redis++.h>

#include <iostream>

#ifdef __cplusplus
extern "C" {
#endif

sw::redis::Redis *redis_client;

void redis_init() {
    redis_client = new sw::redis::Redis("tcp://localhost:9999");
    std::cout << "Test Redis client: client->set(testkey, testval)\n"
              << std::flush;
    redis_client->set("testkey", "testval");
    auto val = redis_client->get("testkey");
    if (val)
        std::cout << "client->get(testkey) = " << *val << "\n"
                  << std::flush;
    redis_client->del("testkey");
    std::cout << "Done redis initalization test\n"
              << std::flush;
}

void redis_deinit() {
    free(redis_client);
}

void redis_put_data(char *key, char *data, int len) {
    //std::cout << "[redis_put_data] Put data: " << key << std::string(data).substr(0, len) << "\n"
    //          << std::flush;
    redis_client->set(key, std::string(data).substr(0, len));
}

#ifdef __cplusplus
}
#endif