//
// Created by Luis Ruisinger on 08.10.23.
//

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "uriparser.h"

char* uri_splice(const char* uri)
{
    char* query_ptr = strstr(uri, "?");
    char* fragment_ptr = strstr(uri, "#");

    if (query_ptr == NULL && fragment_ptr != NULL)
        return NULL;

    if (query_ptr == NULL)
        return strdup(uri);

    u_int32_t size = query_ptr - uri;
    char* route = calloc(size + 1, sizeof(char));

    if (route == NULL)
    {
        fprintf(stderr, "an error occured trying to allocate memory"
                        "for the route whilst splicing the uri");
        exit(EXIT_FAILURE);
    }
    strncpy(route, uri, size);

    return route;
}