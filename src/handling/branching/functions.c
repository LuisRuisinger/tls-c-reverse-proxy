//
// Created by Luis Ruisinger on 09.10.23.
//

#include <stdint.h>
#include <stdlib.h>

#include "functions.h"

struct Server* function(struct Server** server, Function function)
{
    uint32_t size  = 0;
    uint32_t index = 0;

    while (server[index++] != NULL)
        size++;

    uint32_t server_index = rand() % size;
    return server[server_index];
}