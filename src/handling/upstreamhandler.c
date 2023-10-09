//
// Created by Luis Ruisinger on 05.10.23.
//

#include <openssl/ssl.h>
#include <openssl/bio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "fieldparser.h"
#include "tls.h"
#include "client.h"
#include "upstreamhandler.h"

char* handle_upstream_write(HTTP_Header* header, struct Server* server, char* buffer)
{
    struct sockaddr_in6 sock_addr;
    server->socket = socket(PF_INET6, SOCK_STREAM, 0);

    memset(&sock_addr, 0, sizeof(sock_addr));
    sock_addr.sin6_family = AF_INET6;
    sock_addr.sin6_port   = htons(server->port);

    if (inet_pton(AF_INET6, server->ip, &sock_addr.sin6_addr) != 1)
    {
        fprintf(stderr, "invalid ip");
        return NULL;
    }

    if (connect(server->socket, (struct sockaddr *) &sock_addr, sizeof(sock_addr)) != 0)
    {
        //
        // extra logic needed here - fallback
        //

        fprintf(stderr, "server not reachable");
        close(server->socket);
        return NULL;
    }

    struct Client* client = malloc(sizeof(struct Client));

    if (server->protocol == HTTPS)
    {
        if ((client->ctx = create_context()) == NULL)
        {
            SSL_shutdown(client->ssl);
            SSL_free(client->ssl);
            SSL_CTX_free(client->ctx);

            close(server->socket);
            free(client);
            return NULL;
        }

        configure_context(client->ctx);
        client->ssl = SSL_new(client->ctx);
        SSL_set_fd(client->ssl, server->socket);

        if (SSL_connect(client->ssl) == -1)
        {
            SSL_shutdown(client->ssl);
            SSL_free(client->ssl);
            SSL_CTX_free(client->ctx);

            close(server->socket);
            free(client);
            return NULL;
        }

        SSL_write(client->ssl, buffer, sizeof(buffer));

        SSL_shutdown(client->ssl);
        SSL_free(client->ssl);
        SSL_CTX_free(client->ctx);
    }
    else {
        write(client->fd, buffer, sizeof(buffer));
    }
}