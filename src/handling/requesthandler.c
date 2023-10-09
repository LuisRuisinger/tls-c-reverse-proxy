//
// Created by Luis Ruisinger on 05.10.23.
//

#include <stdlib.h>
#include <unistd.h>

#include "fieldparser.h"
#include "readhandler.h"
#include "client.h"
#include "run.h"
#include "upstreamhandler.h"
#include "requesthandler.h"

void* handle_request(void* args)
{
    HTTP_Wrapper_struct* wrapper = handle_read(
            ((struct Handler_arg*) args)->client,
            ((struct Handler_arg*) args)->hashmap
    );

    if (wrapper->pos_routes->server[0] != NULL)
        handle_upstream_write(wrapper->header, wrapper->message, wrapper->pos_routes->server[0]);


    close(((struct Handler_arg*) args)->client->fd);

    header_destroy(wrapper->header);

    free(wrapper->message->header);
    free(wrapper->message->body);
    free(wrapper->pos_routes);
    free(wrapper);

    free(args);

    return NULL;
}