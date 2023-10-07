//
// Created by Luis Ruisinger on 06.10.23.
//

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "setup.h"
#include "hashmap.h"
#include "fieldparser.h"

/*
 * Searches for the field inside the unparsed http request.
 * If the field exists it gets returned as allocated str.
 * Else null will be returned.
 * Does not consume the http request.
 */

static char* parse_auth_field(const char* header, const char* field)
{
    char* auth = strstr(header, field);
    if (auth == NULL)
        return NULL;

    auth += strlen(field);
    char *end = strchr(auth, '\r');

    if (end != NULL)
    {
        size_t length = end - auth;

        char* token = (char*) calloc(length + 1, sizeof(char));
        if (token == NULL)
            return NULL;

        strncpy(token, auth, length);
        return token;
    }
    return NULL;
}

/*
 * Tries to detect the http method in the unparsed http request.
 * If the method is not parsable (e.g. malicious request) BADCODE will be returned.
 * Does not consume the http request.
 */

static Method parse_method(const char* field)
{
    if (strlen(field) == 3 && memcmp(field, "GET", 3) == 0)
        return GET;
    if (strlen(field) == 3 && memcmp(field, "PUT", 3) == 0)
        return PUT;
    if (strlen(field) == 4 && memcmp(field, "POST", 3) == 0)
        return POST;
    if (strlen(field) == 6 && memcmp(field, "DELETE", 6) == 0)
        return DELETE;
    return BADCODE;
}

/*
 * Consumes the read http header request stored and allocated as buffer
 * in the caller of this function.
 * The buffer will be parsed into a HTTP_Header struct containing all the
 * important fields of the request.
 * The http request will be destroyed at the end of the function.
 */

HTTP_Header* parse_fields(char* buffer)
{
    char* method  = calloc(BUFFER_SIZE, sizeof(char));
    char* route   = calloc(BUFFER_SIZE, sizeof(char));
    char* version = calloc(BUFFER_SIZE, sizeof(char));

    HTTP_Header* header = malloc(sizeof(HTTP_Header));

    if (header == NULL || method == NULL || route == NULL || version == NULL)
    {
        if (header  != NULL) free(header);
        if (method  != NULL) free(method);
        if (route   != NULL) free(route);
        if (version != NULL) free(version);
        return NULL;
    }

    if (sscanf(buffer, "%s %s %s", method, route, version) != 3)
    {
        free(header);
        free(method);
        free(route);
        free(version);
        return NULL;
    }

    header->method  = parse_method(method);
    header->version = version;
    header->route   = route;

    header->ips     = NULL;
    header->type    = NONE;

    fprintf(stdout, "request head: %s\n%s\n%s\n", method, version, route);
    free(method);

    header->auth   = parse_auth_field(buffer, "Authorization: ");
    header->cookie = parse_auth_field(buffer, "Cookie: ");
    header->accept = parse_auth_field(buffer, "Accept: ");

    fprintf(stdout, "request head: %s\n%s\n%s\n", header->auth, header->cookie, header->accept);
    free(buffer);

    return header;
}

/*
 * Consumes an allocated HTTP_Header struct and destroys its members
 * and then itself.
 */

void header_destroy(HTTP_Header* header)
{
    if (header == NULL)
        return;

    free(header->version);
    free(header->route);

    if (header->auth   != NULL) free(header->auth);
    if (header->cookie != NULL) free(header->cookie);
    if (header->accept != NULL) free(header->accept);

    free(header);
}

