//
// Created by Luis Ruisinger on 06.10.23.
//

#ifndef C_REVERSE_PROXY_FIELDPARSER_H
#define C_REVERSE_PROXY_FIELDPARSER_H

typedef enum
{
    GET, POST, PUT, DELETE, BADCODE
} Method;

typedef struct
{
    Method method;
    char* version;
    char* route;
    char* cookie;
    char* auth;
    char* accept;

    Type type;
    Code code;
    char** ips;
} HTTP_Header;

HTTP_Header* parse_fields(const char* buffer);
void header_destroy(HTTP_Header* header);

#endif //C_REVERSE_PROXY_FIELDPARSER_H
