//
// Created by Luis Ruisinger on 09.10.23.
//

#ifndef C_REVERSE_PROXY_FUNCTIONS_H
#define C_REVERSE_PROXY_FUNCTIONS_H

#include "setup.h"

typedef enum
{
    RANDOM, AGENT
} Function;

struct Server* function(struct Server** server, Function function);

#endif //C_REVERSE_PROXY_FUNCTIONS_H
