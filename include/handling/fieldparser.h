//
// Created by Luis Ruisinger on 06.10.23.
//

#ifndef C_REVERSE_PROXY_FIELDPARSER_H
#define C_REVERSE_PROXY_FIELDPARSER_H

char* parse_auth_field(char* header, char* field);
char** parse_fields(char* header);
bool isfile(char* route);

#endif //C_REVERSE_PROXY_FIELDPARSER_H
