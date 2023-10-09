//
// Created by Luis Ruisinger on 05.10.23.
//

#include <unistd.h>
#include <netinet/in.h>
#include <string.h>

#include "hashmap.h"
#include "client.h"
#include "setup.h"
#include "tpool.h"
#include "tls.h"
#include "run.h"

#include "parserwrapper.h"
#include "fieldparser.h"
#include "requesthandler.h"
#include "readhandler.h"

static char* realloc_buffer(char* ptr, int32_t len)
{
    char* new_ptr = calloc(BUFFER_SIZE * len, sizeof(char));
    if (new_ptr == NULL)
    {
        free(ptr);
        return NULL;
    }

    memcpy(new_ptr, ptr, strlen(ptr));
    free(ptr);
    return new_ptr;
}


HTTP_Header* handle_read(struct Client* client, struct Hashmap* hashmap)
{
    ssize_t rval;
    ssize_t total = 0;

    int8_t alloc_read = 1;

    struct timeval timeout = {TIMEOUT, 0};
    fd_set readfds;

    time_t start_time;

    char* buffer  = calloc(BUFFER_SIZE, sizeof(char));

    if (buffer == NULL)
        return NULL;

    start_time = time(NULL);
    while (1)
    {
        //
        // timeout for sending empty requests
        //

        FD_ZERO(&readfds);
        FD_SET(client->fd, &readfds);

        if (select(client->fd + 1, &readfds, NULL, NULL, &timeout) < 1)
        {
            free(buffer);
            return NULL;
        }

        //
        // read error or timeout while slowly reading
        //

        if (client->protocol == HTTPS)
            rval = SSL_read(client->ssl, buffer + total, BUFFER_SIZE * alloc_read - total);
        else {
            rval = read(client->fd, buffer + total, BUFFER_SIZE * alloc_read - total);
        }

        if (rval == -1 || (int) difftime(start_time, time(NULL)) >= TIMEOUT)
        {
            free(buffer);
            return NULL;
        }

        total += rval;
        if (strstr(buffer, "\r\n\r\n") != NULL)
            break;

        if (total >= BUFFER_SIZE * alloc_read)
        {
            buffer = realloc_buffer(buffer, ++alloc_read);
            if (buffer == NULL)
                return NULL;
        }
    }

#if DEBUG

    fprintf(stdout, "read from client in %f\n\n", difftime(start_time, time(NULL)));

#endif

    // parsing fields and destroying buffer

    HTTP_Header* header = parse_fields(buffer);

    if (header == NULL || header->method == BADCODE)
        return NULL;

    if (header->method != GET && header->accept == NULL)
    {
        header->type = header->method == GET ? STATICFILE : PROTOCOL;
        header->ips = hashmap->get(hashmap, header->route, header->type);
    }
    else {
        for (int n = 0;; n++)
        {
            if (header->accept[n] == NULL || header->ips != NULL)
                break;

            if (header->accept != NULL &&
                strstr(header->accept[n]->mime, "text") != NULL)
                header->type = STATICFILE;
            if (header->accept != NULL &&
                strstr(header->accept[n]->mime, "application/json") != NULL)
                header->type = PROTOCOL;

            header->pos_routes = hashmap->get(hashmap, header->route, header->type);
        }
    }

    if (header->pos_routes == NULL)
        header->code = BADREQUEST;
    else {
        header->code = OK;
    }

#if DEBUG

    fprintf(stdout, "type    : %s\n", header->type == STATICFILE ? "STATICFILE" : "PROTOCOL");
    fprintf(stdout, "code    : %s\n\n\n\n", header->code == OK ? "OK" : "BADREQUEST");

#endif

    return header;
}