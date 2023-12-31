//
// Created by Luis Ruisinger on 05.10.23.
//

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>

#include "hashmap.h"

static uint32_t fnv1a_hash(const char *str)
{
    uint32_t hash = 2166136261u;
    while (*str)
    {
        hash ^= (uint32_t)*str++;
        hash *= 16777619u;
    }
    return hash;
}

void hashmap_add(struct Hashmap* hashmap, char* key, Type type, struct Server* server)
{
    uint32_t index = fnv1a_hash(key) % hashmap->size;
    struct Linkedlist_hashmap* cur = hashmap->buckets[index];

    if (hashmap->buckets[index] == NULL)
    {
        hashmap->buckets[index] = malloc(sizeof(struct Linkedlist_hashmap));
        cur =  hashmap->buckets[index];
        assert(cur != NULL);

        cur->key      = strdup(key);
        cur->initial  = malloc(sizeof(struct Element));
        assert(cur->initial != NULL);

        cur->type     = type;
        cur->next     = NULL;

        cur->initial->value = server;
        cur->initial->next  = NULL;
        return;
    }

    while (cur->next != NULL)
    {
        if (strlen(cur->key) == strlen(key) &&
            memcmp(cur->key, key, strlen(key)) == 0 &&
            cur->type == type)
        {
            struct Element* cur_el = cur->initial;
            while (cur_el->next != NULL)
                cur_el = cur_el->next;

            cur_el->next = malloc(sizeof(struct Element));
            assert(cur_el->next != NULL);

            cur_el->next->value = server;
            cur_el->next->next  = NULL;
            return;
        }
        cur = cur->next;
    }

    cur->next = malloc(sizeof(struct Linkedlist_hashmap));
    assert(cur->next != NULL);

    cur->next->key      = strdup(key);
    cur->next->initial  = malloc(sizeof(struct Element));
    assert(cur->next->initial != NULL);

    cur->next->type     = type;
    cur->next->next     = NULL;

    cur->next->initial->value = server;
    cur->next->initial->next  = NULL;
}

void hashmap_add_all(struct Hashmap* hashmap, char* key, Type type, struct Server* f_server, ...)
{
    va_list argList;
    va_start(argList, f_server);

    struct Server* current = f_server;
    while (current != NULL)
    {
        hashmap_add(hashmap, key, type, current);
        current = va_arg(argList, struct Server*);
    }

    va_end(argList);
}

struct Server** hashmap_get(struct Hashmap* hashmap, char* key, Type type)
{
    uint32_t index = fnv1a_hash(key) % hashmap->size;
    struct Linkedlist_hashmap* cur = hashmap->buckets[index];

    if (cur == NULL)
        return NULL;

    while (cur != NULL)
    {
        if (strlen(cur->key) == strlen(key) &&
            memcmp(cur->key, key, strlen(key)) == 0 &&
            cur->type == type)
        {
            uint32_t size = 0;
            struct Element* element = cur->initial;

            while (element != NULL)
            {
                size++;
                element = element->next;
            }

            struct Server** server = calloc(size + 1, sizeof(struct Server*));
            assert(server != NULL);

            element = cur->initial;
            for (int n = 0; n < size; n++)
            {
                server[n] = element->value;
                element = element->next;
            }

            server[size] = NULL;
            return server;
        }
        cur = cur->next;
    }
    return NULL;
}

struct Hashmap* hashmap_init(uint32_t size)
{
    struct Hashmap* hashmap = malloc(sizeof(struct Hashmap));
    assert(hashmap != NULL);

    hashmap->add     = hashmap_add;
    hashmap->add_all = hashmap_add_all;
    hashmap->get     = hashmap_get;
    hashmap->size    = MAX(size, MINSIZE);
    hashmap->buckets = calloc(hashmap->size, sizeof(struct Linkedlist_hashmap*));
    assert(hashmap->buckets != NULL);

    for (int n = 0; n < hashmap->size; n++)
        hashmap->buckets[n] = NULL;

    return hashmap;
}
