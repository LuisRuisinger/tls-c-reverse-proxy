//
// Created by Luis Ruisinger on 05.10.23.
//

#ifndef C_REVERSE_PROXY_HASHMAP_H
#define C_REVERSE_PROXY_HASHMAP_H

#include <stdint.h>

#include "setup.h"

#define MAX(x, y) (x > y ? x : y)
#define MINSIZE 16

typedef enum
{
    STATICFILE, PROTOCOL, NONE
} Type;

struct Element
{
    char* value;
    struct Element* next;
};

struct Linkedlist
{
    char* key;
    Type type;
    enum Protocol protocol;
    struct Element* initial;
    struct Linkedlist* next;
};

typedef struct {
    char** ips;
    enum Protocol protocol;
} Routes;

struct Hashmap
{
    void    (*add)     (struct Hashmap* hashmap, char* key, Type type, enum Protocol protocol, char* value);
    void    (*add_all) (struct Hashmap* hashmap, char* key, Type type, enum Protocol protocol, char* f_value, ...);
    Routes* (*get)     (struct Hashmap* hashmap, char* key, Type type);

    uint32_t size;
    struct Linkedlist** buckets;
};

struct Hashmap* hashmap_init(uint32_t size);

#endif //C_REVERSE_PROXY_HASHMAP_H
