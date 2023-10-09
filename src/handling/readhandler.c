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

char* read_header(struct Client* client)
{

}


HTTP_Wrapper_struct* handle_read(struct Client* client, struct Hashmap* hashmap)
{
    ssize_t rval;
    ssize_t total = 0;

    int8_t alloc_read = 1;

    struct timeval timeout = {TIMEOUT, 0};
    fd_set readfds;

    time_t start_time;

    char* buffer_header  = calloc(BUFFER_SIZE, sizeof(char));
    if (buffer_header == NULL)
        return NULL;

    HTTP_Message* message = malloc(sizeof(HTTP_Message));
    if (message == NULL)
        return NULL;

    // read the header

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
            free(buffer_header);
            return NULL;
        }

        //
        // read error or timeout while slowly reading
        //

        if (client->protocol == HTTPS)
            rval = SSL_read(client->ssl, buffer_header + total, BUFFER_SIZE * alloc_read - total);
        else {
            rval = read(client->fd, buffer_header + total, BUFFER_SIZE * alloc_read - total);
        }

        if (rval == -1 || (int) difftime(start_time, time(NULL)) >= TIMEOUT)
        {
            free(buffer_header);
            return NULL;
        }

        total += rval;
        if (strstr(buffer_header, "\r\n\r\n") != NULL)
            break;

        if (total >= BUFFER_SIZE * alloc_read)
        {
            buffer_header = realloc_buffer(buffer_header, ++alloc_read);
            if (buffer_header == NULL)
                return NULL;
        }
    }
    message->header = buffer_header;

#if DEBUG

    fprintf(stdout, "read from client in %f\n\n", difftime(start_time, time(NULL)));

#endif

    // parsing fields and destroying buffer_header

    HTTP_Header* header = parse_fields(buffer_header);
    if (header == NULL || header->method == BADCODE)
    {
        header_destroy(header);
        free(buffer_header);
        return NULL;
    }

    // read the body

    if (header->length != 0)
    {
        char* buffer_body  = calloc(header->length + 1, sizeof(char));
        if (buffer_body == NULL)
        {
            free(buffer_header);
            header_destroy(header);
            return NULL;
        }

        start_time = time(NULL);
        while (strlen(buffer_body) != header->length)
        {
            if (client->protocol == HTTPS)
                rval = SSL_read(client->ssl, buffer_body + total, BUFFER_SIZE * alloc_read - total);
            else {
                rval = read(client->fd, buffer_body + total, BUFFER_SIZE * alloc_read - total);
            }

            if (rval == -1 || (int) difftime(start_time, time(NULL)) >= TIMEOUT)
            {
                free(buffer_header);
                header_destroy(header);
                return NULL;
            }
        }
        message->body = buffer_body;
    }

    // find routes

    HTTP_Wrapper_struct* wrapper = malloc(sizeof(HTTP_Wrapper_struct));
    if (wrapper == NULL)
    {
        header_destroy(header);
        return NULL;
    }

    wrapper->header  = header;
    wrapper->message = message;

    if (header->method != GET && header->accept == NULL)
    {
        wrapper->type = header->method == GET ? STATICFILE : PROTOCOL;
        wrapper->pos_routes = hashmap->get(hashmap, header->route, wrapper->type);
    }
    else {
        for (int n = 0;; n++)
        {
            if (header->accept[n] == NULL || wrapper->pos_routes != NULL)
                break;

            if (header->accept != NULL &&
                strstr(header->accept[n]->mime, "text") != NULL)
                wrapper->type = STATICFILE;
            if (header->accept != NULL &&
                strstr(header->accept[n]->mime, "application/json") != NULL)
                wrapper->type = PROTOCOL;

            wrapper->pos_routes = hashmap->get(hashmap, header->route, wrapper->type);
        }
    }

    if (wrapper->pos_routes == NULL)
        wrapper->code = BADREQUEST;
    else {
        wrapper->code = OK;
    }

#if DEBUG

    fprintf(stdout, "type    : %s\n", wrapper->type == STATICFILE ? "STATICFILE" : "PROTOCOL");
    fprintf(stdout, "code    : %s\n\n\n\n", wrapper->code == OK ? "OK" : "BADREQUEST");

#endif

    return wrapper;
}