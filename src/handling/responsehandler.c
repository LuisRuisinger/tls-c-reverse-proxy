//
// Created by Luis Ruisinger on 10.10.23.
//

#include <stdlib.h>
#include <unistd.h>

#include "pendingpool.h"
#include "functions.h"
#include "fieldparser.h"
#include "client.h"
#include "run.h"
#include "upstreamhandler.h"
#include "requesthandler.h"
#include "responsehandler.h"

void* handle_response(void* args)
{
    free(args);
    return NULL;
}