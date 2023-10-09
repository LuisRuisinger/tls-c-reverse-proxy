//
// Created by Luis Ruisinger on 06.10.23.
//

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <uuid/uuid.h>
#include <assert.h>

#include "setup.h"
#include "hashmap.h"
#include "fieldparser.h"
#include "uriparser.h"

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

static Accept** parse_accept_field(char* field, uint32_t* format)
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
            *format = MAX(*format, size);

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
 * Simple HTTP_Header visualisation.
 * Does not consume the HTTP_Header;
 */

static void header_print(HTTP_Header* header, uint32_t format)
{
    fprintf(stdout,
            "method  : %d\n"
            "version : %s\n"
            "uri     : %s\n"
            "route   : %s\n",
            header->method,
            header->version,
            header->uri,
            header->route
    );

    fprintf(stdout,
            "length  : %d\n"
            "auth    : %s\n"
            "cookie  : %s\n",
            header->length,
            header->auth,
            header->cookie
    );
    fprintf(stdout, "mime    : ");

    for (int n = 0; n < format + 11; n++)
        fprintf(stdout, "-");
    fprintf(stdout, "\n");

    if (header->accept != NULL)
        for (int n = 0;; n++)
        {
            if (header->accept[n] == NULL)
                break;

            fprintf(stdout, "          %s", header->accept[n]->mime);
            for (int m = 0; m < format - strlen(header->accept[n]->mime); m++)
                fprintf(stdout, " ");

            fprintf(stdout, " - %f\n", header->accept[n]->pref);
        }

    fprintf(stdout, "          ");
    for (int n = 0; n < format + 11; n++)
        fprintf(stdout, "-");
    fprintf(stdout, "\n");
}

/*
 * Parses the read http header request stored and allocated as buffer
 * in the caller of this function.
 * The buffer will be parsed into a HTTP_Header struct containing all the
 * important fields of the request.
 * The http request won't be consumed after parsing;
 */

HTTP_Header* parse_fields(char* buffer)
{
    char* method  = calloc(BUFFER_SIZE, sizeof(char));
    char* uri     = calloc(BUFFER_SIZE, sizeof(char));
    char* version = calloc(BUFFER_SIZE, sizeof(char));

    HTTP_Header* header = malloc(sizeof(HTTP_Header));

    if (header == NULL || method == NULL || uri == NULL || version == NULL)
    {
        if (header  != NULL) free(header);
        if (method  != NULL) free(method);
        if (uri     != NULL) free(uri);
        if (version != NULL) free(version);
        return NULL;
    }

    if (sscanf(buffer, "%s %s %s", method, uri, version) != 3)
    {
        free(header);
        free(method);
        free(uri);
        free(version);
        return NULL;
    }

    header->uuid = malloc(sizeof(uuid_t));
    assert(header->uuid != NULL);

    // freeing of version and uri not needed here - happens in header_destroy

    header->method  = parse_method_field(method);
    header->route   = uri_splice(uri);
    header->version = version;
    header->uri     = uri;

    header->auth    = parse_auth_field(buffer, "Authorization: ");
    header->cookie  = parse_auth_field(buffer, "Cookie: ");

    char* length    = parse_auth_field(buffer, "Content-Length: ");
    if (length != NULL)
    {
        header->length = atoi(length);
        free(length);
    }
    else {
        header->length = 0;
    }

    char* uuid = parse_auth_field(buffer, "UUID: ");
    if (uuid != NULL)
    {
        uuid_parse(uuid, *(header->uuid));
        free(uuid);
    }
    else {
        uuid_generate_random(*(header->uuid));
    }

    char* accept_field = parse_auth_field(buffer, "Accept: ");
    uint32_t format    = 0;

    header->accept = parse_accept_field(accept_field, &format);

#if DEBUG

    header_print(header, format);

#endif

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
    free(header->uri);
    free(header->route);

    if (header->auth   != NULL) free(header->auth);
    if (header->cookie != NULL) free(header->cookie);
    if (header->accept != NULL)
        while (*(header->accept) != NULL)
            free((*(header->accept++))->mime);

    free(header);
}

