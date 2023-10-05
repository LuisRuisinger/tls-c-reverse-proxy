#include <stdio.h>

#include "../include/setup.h"
#include "../include/run.h"

#define TESTIPV6 "::1"
#define TESTIPV4 "127.0.0.1"

int main() {
    struct Server* server = server_init(HTTPS, IPv6, TESTIPV6, 8080);
    server_run(server);
}
