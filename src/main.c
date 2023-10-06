#include <stdio.h>

#include "../include/setup.h"
#include "../include/run.h"
#include "hashmap.h"

#define TESTIPV6 "::1"
#define TESTIPV4 "127.0.0.1"

int main() {
    struct Server* server = server_init(HTTP, IPv6, TESTIPV6, 8080);
    struct Hashmap* hashmap = hashmap_init(16);
    server_run(server, hashmap);
}
