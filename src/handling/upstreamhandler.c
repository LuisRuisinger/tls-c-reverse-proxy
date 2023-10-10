//
// Created by Luis Ruisinger on 05.10.23.
//

#include <openssl/ssl.h>
#include <openssl/bio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>

#include "fieldparser.h"
#include "tls.h"
#include "client.h"
#include "upstreamhandler.h"

static char* build_header(HTTP_Header* header)
{
    char* buffer = calloc(BUFFER_SIZE * 4, sizeof(char));
    assert(buffer != NULL);

    switch (header->method) {

        case GET:     strcat(buffer, "GET ");    break;
        case POST:    strcat(buffer, "POST ");   break;
        case PUT:     strcat(buffer, "PUT ");    break;
        case DELETE:  strcat(buffer, "DELETE "); break;
        case BADCODE: return NULL;
    }

    strcat(buffer, header->uri);
    strcat(buffer, " HTTP/1.1\r\nHost: ");
    strcat(buffer, header->host);
    strcat(buffer, "\r\nConnection: keep-alive\r\nAccept: ");
    strcat(buffer, header->accept[0]->mime);

    if (header->auth != NULL)
    {
        strcat(buffer, "\r\nAuthentication: ");
        strcat(buffer, header->auth);
    }

    if (header->cookie != NULL)
    {
        strcat(buffer, "\r\nCookie: ");
        strcat(buffer, header->cookie);
    }


    strcat(buffer, "\r\nContent-Length: ");
    char num[16];
    sprintf(num, "%d", header->length);
    strcat(buffer, num);

    strcat(buffer, "\r\nUUID: ");
    strcat(buffer, (const char *) header->uuid);
    strcat(buffer, "\r\n\r\n");

    return buffer;
}

int32_t handle_upstream_write(HTTP_Header* header, char* body, struct Server* server)
{
    ssize_t rval;
    struct sockaddr_in6 sock_addr;
    server->socket = socket(PF_INET6, SOCK_STREAM, 0);

    memset(&sock_addr, 0, sizeof(sock_addr));
    sock_addr.sin6_family = AF_INET6;
    sock_addr.sin6_port   = htons(server->port);

    if (inet_pton(AF_INET6, server->ip, &sock_addr.sin6_addr) != 1)
    {
        fprintf(stderr, "invalid ip");
        return EXIT_FAILURE;
    }

    if (connect(server->socket, (struct sockaddr *) &sock_addr, sizeof(sock_addr)) != 0)
    {
        //
        // extra logic needed here - fallback
        //

        fprintf(stderr, "server not reachable");
        close(server->socket);
        return -1;
    }

    fprintf(stdout, "connected to : %s : %d\n\n", server->ip, server->port);

    struct Client* client = malloc(sizeof(struct Client));
    if (client == NULL)
        return EXIT_FAILURE;

    char* buffer = build_header(header);
    if (buffer == NULL)
        return EXIT_FAILURE;

    if (server->protocol == HTTPS)
    {
        if ((client->ctx = create_context()) == NULL)
        {
            SSL_shutdown(client->ssl);
            SSL_free(client->ssl);
            SSL_CTX_free(client->ctx);

            close(server->socket);
            free(client);
            return EXIT_FAILURE;
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
            return EXIT_FAILURE;
        }

        SSL_write(client->ssl, buffer, strlen(buffer) + 1);

        if (body != NULL)
            SSL_write(client->ssl, body, strlen(body) + 1);

        SSL_shutdown(client->ssl);
        SSL_free(client->ssl);
        SSL_CTX_free(client->ctx);
    }
    else {
        rval = write(server->socket, buffer, strlen(buffer) + 1);
        if (rval == -1)
            perror((const char *) &errno);

        if (body != NULL)
        {
            rval = write(server->socket, body, strlen(body) + 1);
            if (rval == -1)
                perror((const char *) &errno);
        }
    }
    return EXIT_SUCCESS;
}