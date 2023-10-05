//
// Created by Luis Ruisinger on 05.10.23.
//

#ifndef C_REVERSE_PROXY_ROUTING_H
#define C_REVERSE_PROXY_ROUTING_H

typedef struct {} Endpoints;
typedef struct {} Files;

typedef struct {
    Endpoints endpoints;
    Files files;
} Routes;

int32_t add_endpoint(char* endpoint, char* ip);
int32_t add_file(char* file, char* ip);

#endif //C_REVERSE_PROXY_ROUTING_H
