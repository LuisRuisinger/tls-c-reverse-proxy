//
// Created by Luis Ruisinger on 06.10.23.
//

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "setup.h"
#include "hashmap.h"
#include "handling/parserwrapper.h"
#include "handling/fieldparser.h"

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

HTTP_Header* parse_fields(const char* buffer)
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

    free(method);
    fprintf(stdout, "request head: %s %s %s\n", method, version, route);

    header->auth   = parse_auth_field(buffer, "Authorization: ");
    header->cookie = parse_auth_field(buffer, "Cookie: ");
    header->accept = parse_auth_field(buffer, "Accept: ");

    fprintf(stdout, "request head: %s %s %s\n", header->auth, header->cookie, header->accept);
    return header;
}

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

