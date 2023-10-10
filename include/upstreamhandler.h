//
// Created by Luis Ruisinger on 05.10.23.
//

#ifndef C_REVERSE_PROXY_UPSTREAMHANDLER_H
#define C_REVERSE_PROXY_UPSTREAMHANDLER_H

int32_t handle_upstream_write(HTTP_Header* header, char* body, struct Server* server);

#endif //C_REVERSE_PROXY_UPSTREAMHANDLER_H
