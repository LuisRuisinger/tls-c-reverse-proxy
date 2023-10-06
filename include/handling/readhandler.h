//
// Created by Luis Ruisinger on 05.10.23.
//

#ifndef C_REVERSE_PROXY_READHANDLER_H
#define C_REVERSE_PROXY_READHANDLER_H

HTTP_Header* handle_read(struct Client* client, struct Hashmap* hashmap);

#endif //C_REVERSE_PROXY_READHANDLER_H
