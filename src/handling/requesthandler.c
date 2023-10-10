//
// Created by Luis Ruisinger on 05.10.23.
//

#include <stdlib.h>
#include <unistd.h>

#include "pendingpool.h"
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

    if (wrapper == NULL)
    {
        close(((struct Handler_arg*) args)->client->fd);
        return NULL;
    }

    //
    // choose entry - load balancing functions
    //
    // also handle BADREQUESTS
    // handle timeouts ... etc. future work hehehehaw
    //

    int32_t ret;
    struct Server* server = function(wrapper->server, RANDOM);

    ret = handle_upstream_write(wrapper->header, wrapper->body, server);
    if (ret == EXIT_FAILURE)
    {
        close(((struct Handler_arg*) args)->client->fd);
        header_destroy(wrapper->header);

        free(wrapper->body);
        free(wrapper->server);
        free(wrapper);
        free(args);

        return NULL;
    }

    ret = ((struct  Handler_arg*) args)->pool->add(
            wrapper, ((struct  Handler_arg*) args)->client, server
    );
    if (ret == EXIT_FAILURE)
    {
        close(((struct Handler_arg*) args)->client->fd);
        header_destroy(wrapper->header);

        free(wrapper->body);
        free(wrapper->server);
        free(wrapper);
        free(args);

        return NULL;
    }

    free(args);
    return NULL;
}