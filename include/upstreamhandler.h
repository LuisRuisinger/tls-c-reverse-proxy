//
// Created by Luis Ruisinger on 05.10.23.
//

#ifndef C_REVERSE_PROXY_UPSTREAMHANDLER_H
#define C_REVERSE_PROXY_UPSTREAMHANDLER_H

char* handle_upstream_write(HTTP_Header* header, HTTP_Message* message, struct Server* server);

#endif //C_REVERSE_PROXY_UPSTREAMHANDLER_H
