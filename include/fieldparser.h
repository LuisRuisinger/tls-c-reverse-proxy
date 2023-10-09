//
// Created by Luis Ruisinger on 06.10.23.
//

#ifndef C_REVERSE_PROXY_FIELDPARSER_H
#define C_REVERSE_PROXY_FIELDPARSER_H

#include "hashmap.h"
#include "parserwrapper.h"

typedef enum
{
    GET, POST, PUT, DELETE, BADCODE
} Method;

typedef struct
{
    char* mime;
    double pref;
} Accept;

typedef struct
{
    Method method;
    char* version;
    char* uri;
    char* route;
    char* cookie;
    char* auth;
    Accept** accept;

    Type type;
    Code code;
    Routes* pos_routes;
} HTTP_Header;

HTTP_Header* parse_fields(char* buffer);
void header_destroy(HTTP_Header* header);

#endif //C_REVERSE_PROXY_FIELDPARSER_H
