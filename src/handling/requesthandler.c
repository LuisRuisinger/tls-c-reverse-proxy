//
// Created by Luis Ruisinger on 05.10.23.
//

#include <stdlib.h>
#include <unistd.h>

#include "handling/fieldparser.h"
#include "handling/readhandler.h"
#include "client.h"
#include "run.h"
#include "handling/requesthandler.h"

void* handle_request(void* args)
{
    HTTP_Header* header = handle_read(
            ((struct Handler_arg*) args)->client,
            ((struct Handler_arg*) args)->hashmap
    );

    close(((struct Handler_arg*) args)->client->fd);

    free(header);
    free(args);

    return NULL;
}