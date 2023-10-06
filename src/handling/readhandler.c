//
// Created by Luis Ruisinger on 05.10.23.
//

#include <unistd.h>
#include <netinet/in.h>
#include <string.h>

#include "setup.h"
#include "tpool.h"
#include "client.h"
#include "tls.h"
#include "run.h"

#include "handling/requesthandler.h"
#include "handling/readhandler.h"
#include "handling/fieldparser.h"
#include "handling/parserwrapper.h"

static void clean_mem(char** ptr_list, int32_t len)
{
    for (int n = 0; n < len; n++)
        free(ptr_list[n]);
}

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


char* handle_read(struct Client* client, struct Hashmap* hashmap)
{
    ssize_t rval;
    ssize_t total = 0;

    int8_t alloc_read = 1;

    struct timeval timeout = {TIMEOUT, 0};
    fd_set readfds;

    time_t start_time;

    char* buffer  = calloc(BUFFER_SIZE, sizeof(char));
    char* method  = calloc(BUFFER_SIZE, sizeof(char));
    char* route   = calloc(BUFFER_SIZE, sizeof(char));
    char* version = calloc(BUFFER_SIZE, sizeof(char));

    if (buffer == NULL || method == NULL || route == NULL || version == NULL)
    {
        close(client->fd);
        return NULL;
    }

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
            char* arr[] = {method, route, version, buffer};
            clean_mem(arr, 4);
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
            char* arr[] = {method, route, version, buffer};
            clean_mem(arr, 4);
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

    fprintf(stdout, "read from client in : %f\n", difftime(start_time, time(NULL)));

    if (sscanf(buffer, "%s %s %s", method, route, version) != 3)
    {
        char* arr[] = {method, route, version, buffer};
        clean_mem(arr, 4);
        return NULL;
    }

    fprintf(stdout, "request head: %s %s %s\n", method, version, route);

    char** fields = parse_fields(buffer);
    fprintf(stdout, "the auth field : --%s--\n", *fields);



    char* arr[] = {method, route, version, buffer};
    clean_mem(arr, 4);

    free(fields);

    return NULL;
}