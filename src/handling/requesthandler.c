//
// Created by Luis Ruisinger on 05.10.23.
//

#include <stdlib.h>
#include <unistd.h>

#include "functions.h"
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

    //
    // choose entry - load balancing functions
    //
    // also handle BADREQUESTS
    //

    struct Server* server = function(wrapper->server, RANDOM);
    handle_upstream_write(wrapper->header, wrapper->message, server);

    //
    // caching ... response ... etc ...
    //

    close(((struct Handler_arg*) args)->client->fd);

    header_destroy(wrapper->header);

    free(wrapper->message->header);
    free(wrapper->message->body);
    free(wrapper->server);
    free(wrapper);

    free(args);

    return NULL;
}