#include <stdio.h>

#include "../include/setup.h"
#include "../include/run.h"
#include "hashmap.h"

#define TESTIPV6 "::1"
#define TESTIPV4 "127.0.0.1"

int main() {
    struct Server* server   = server_init(HTTP, IPv6, TESTIPV6, 9090);
    struct Hashmap* hashmap = hashmap_init(16);
    struct Server backend   = {
            .ip = "::1",
            .port = 8080,
            .protocol = HTTP,
            .version = IPv6
    };

    hashmap->add(hashmap, "/", STATICFILE, &backend);
    server_run(server, hashmap);
}
