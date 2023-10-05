//
// Created by Luis Ruisinger on 05.10.23.
//

#ifndef C_REVERSE_PROXY_RUN_H
#define C_REVERSE_PROXY_RUN_H

struct Handler_arg {
    struct Client* client;
    struct Hashmap* hashmap;
};

void server_run(struct Server* server, struct Hashmap* hashmap);

#endif //C_REVERSE_PROXY_RUN_H
