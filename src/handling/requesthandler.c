//
// Created by Luis Ruisinger on 05.10.23.
//

#include <stdlib.h>
#include "handling/requesthandler.h"
#include "run.h"

void* handle_request(void* args)
{
    struct Handler_arg arg = *(struct Handler_arg*) args;
    free(args);


}