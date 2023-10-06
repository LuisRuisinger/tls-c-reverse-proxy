//
// Created by Luis Ruisinger on 05.10.23.
//

#ifndef C_REVERSE_PROXY_CLIENT_H
#define C_REVERSE_PROXY_CLIENT_H

#include <openssl/ssl.h>
#include <netinet/in.h>

#include "setup.h"

typedef struct Client
{
    int32_t fd;
    in_port_t sin_port;
    union
    {
        struct in6_addr sin6_addr;
        struct in_addr sin_addr;
    };
    enum Protocol protocol;
    SSL_CTX* ctx;
    SSL* ssl;
} client;


#endif //C_REVERSE_PROXY_CLIENT_H
