//
// Created by Luis Ruisinger on 05.10.23.
//

#ifndef C_REVERSE_PROXY_TLS_H
#define C_REVERSE_PROXY_TLS_H

SSL_CTX *create_context();
void configure_context(SSL_CTX *ctx);

#endif //C_REVERSE_PROXY_TLS_H
