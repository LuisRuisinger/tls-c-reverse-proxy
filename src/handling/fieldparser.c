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
 * Consumes the method field of the http request.
 */

static Method parse_method_field(char* field)
{
    if (strlen(field) == 3 && memcmp(field, "GET", 3) == 0)
    {
        free(field);
        return GET;
    }
    if (strlen(field) == 3 && memcmp(field, "PUT", 3) == 0)
    {
        free(field);
        return PUT;
    }
    if (strlen(field) == 4 && memcmp(field, "POST", 3) == 0)
    {
        free(field);
        return POST;
    }
    if (strlen(field) == 6 && memcmp(field, "DELETE", 6) == 0)
    {
        free(field);
        return DELETE;
    }

    free(field);
    return BADCODE;
}

/*
 * Parses the accept_field str into single mime fields with their respective q-value.
 * Because the existence of a q-value is not guaranteed it will be set to 1.0 per default.
 * Consumes the accept_field str of the http request.
 */

static Accept** parse_accept_field(char* field)
{
    if (field == NULL)
        return NULL;

    uint32_t count   = 0;
    uint32_t index   = 0;
    uint32_t l_split = 0;
    uint32_t size;
    bool flag = false;

    char* ptr = (char*) field;
    while (*ptr)
        if (*(ptr++) == ',')
            count++;

    Accept** accept_elements = calloc(count + 2, sizeof(Accept*));

    if (accept_elements == NULL)
        return NULL;
    accept_elements[count + 1] = NULL;

    for (int n = 0; n < strlen(field) + 1; n++)
    {
        if (accept_elements[index] == NULL)
        {
            accept_elements[index] = malloc(sizeof(Accept));
            if (accept_elements[index] == NULL)
                return NULL;
        }

        if ((field[n] == ';') ||
            (field[n] == ',' && !flag) ||
            (field[n] == '\0' && !flag))
        {
            size = n - l_split;
            accept_elements[index]->mime = calloc(size, sizeof(char));
            accept_elements[index]->pref = 1.0;

            if (accept_elements[index]->mime == NULL)
                return NULL;

            strncpy(accept_elements[index]->mime, &field[l_split], size);
            l_split = n + 1;
        }
        else if ((field[n] == ',' && flag) ||
                 (field[n] == '\0' && flag))
        {
            while (field[l_split++] != '=');

            char* endptr = (char*) &field[n];
            accept_elements[index]->pref = strtod(&field[l_split], &endptr);
            l_split = n + 1;
        }

        if (field[n] == ';') flag = true;
        else if (field[n] == ',')
        {
            flag = false;
            index++;
        }
    }

    free(field);
    return accept_elements;
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

    header->method  = parse_method_field(method);
    header->version = strdup(version);
    header->route   = strdup(route);

    header->ips     = NULL;
    header->type    = NONE;

    fprintf(stdout,
            "method  : %d\n"
            "version : %s\n"
            "route   : %s\n",
            header->method,
            header->version,
            header->route
    );

    header->auth   = parse_auth_field(buffer, "Authorization: ");
    header->cookie = parse_auth_field(buffer, "Cookie: ");

    char* accept_field = parse_auth_field(buffer, "Accept: ");
    header->accept = parse_accept_field(accept_field);

    fprintf(stdout,
            "auth    : %s\n"
            "cookie  : %s\n",
            header->auth,
            header->cookie
    );
    fprintf(stdout, "mime    : ----------\n");

    if (header->accept != NULL)
        for (int n = 0;; n++)
        {
            if (header->accept[n] == NULL)
                break;

            fprintf(stdout,
                    "          %s - %f\n",
                    header->accept[n]->mime ,
                    header->accept[n]->pref
            );
        }

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
    if (header->accept != NULL)
        while (*(header->accept++) != NULL)
            free((*(header->accept))->mime);

    free(header);
}

