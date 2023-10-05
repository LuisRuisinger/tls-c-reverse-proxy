//
// Created by Luis Ruisinger on 05.10.23.
//

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "hashmap.h"

#define MAX(x, y) (x > y ? x : y)
#define MINSIZE 16

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

void hashmap_add(char* key, char* value, struct Hashmap* hashmap)
{
    uint32_t index = fnv1a_hash(key);
    struct Linkedlist* cur = hashmap->buckets[index];

    if (cur == NULL)
    {
        cur->key     = strdup(key);
        cur->initial = malloc(sizeof(struct Element));
        cur->next    = NULL;

        if (cur->initial == NULL)
        {
            fprintf(stderr, "memory couldn't be allocated for an element of a bucket");
            exit(EXIT_FAILURE);
        }

        cur->initial->value = strdup(value);
        cur->initial->next  = NULL;
        return;
    }

    while (cur->next != NULL)
    {
        if (strlen(cur->key) == strlen(key) &&
            memcmp(cur->key, key, strlen(key)) == 0)
        {
            struct Element* cur_el = cur->initial;
            while (cur_el->next != NULL)
                cur_el = cur_el->next;

            cur_el->next = malloc(sizeof(struct Element));

            if (cur_el->next == NULL)
            {
                fprintf(stderr, "memory couldn't be allocated for a next element");
                exit(EXIT_FAILURE);
            }

            cur_el->next->value = value;
            cur_el->next->next  = NULL;
            return;
        }
        cur = cur->next;
    }

    cur->next = malloc(sizeof(struct Linkedlist));

    if (cur->next == NULL)
    {
        fprintf(stderr, "memory couldn't be allocated for a next linkedlist");
        exit(EXIT_FAILURE);
    }

    cur->next->key     = key;
    cur->next->initial = malloc(sizeof(struct Element));
    cur->next->next    = NULL;

    if (cur->next->initial == NULL)
    {
        fprintf(stderr, "memory couldn't be allocated for an element of a bucket");
        exit(EXIT_FAILURE);
    }

    cur->next->initial->value = strdup(value);
    cur->next->initial->next  = NULL;
}

char** hashmap_get(char* key, struct Hashmap* hashmap)
{
    uint32_t index = fnv1a_hash(key);
    struct Linkedlist* cur = hashmap->buckets[index];

    if (cur == NULL)
        return NULL;

    while (cur->next != NULL)
    {
        if (strlen(cur->key) == strlen(key) &&
            memcmp(cur->key, key, strlen(key)) == 0)
        {
            uint32_t size = 0;
            struct Element* element = cur->initial;

            while (element != NULL)
            {
                size++;
                element = element->next;
            }

            char** list = calloc(size + 1, sizeof(char*));
            if (list == NULL)
            {
                fprintf(stderr, "memory couldn't be allocated for the list of values");
                exit(EXIT_FAILURE);
            }

            element = cur->initial;
            for (int n = 0; n < size; n++)
            {
                list[n] = element->value;
                element = element->next;
            }

            list[size] = NULL;
            return list;
        }
        cur = cur->next;
    }
    return NULL;
}

struct Hashmap* hashmap_init(uint32_t size)
{
    struct Hashmap* hashmap = malloc(sizeof(struct Hashmap));

    if (hashmap == NULL)
    {
        fprintf(stderr, "memory couldn't be allocated for the hashmap");
        exit(EXIT_FAILURE);
    }

    hashmap->add     = hashmap_add;
    hashmap->get     = hashmap_get;
    hashmap->size    = MAX(size, MINSIZE);
    hashmap->buckets = calloc(hashmap->size, sizeof(struct Linkedlist*));

    if (hashmap->buckets == NULL)
    {
        fprintf(stderr, "memory couldn't be allocated for the buckets of the hashmap");
        exit(EXIT_FAILURE);
    }

    for (int n = 0; n < hashmap->size; n++)
        hashmap->buckets[n] = NULL;

    return hashmap;
}
