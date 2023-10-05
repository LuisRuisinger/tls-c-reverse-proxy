//
// Created by Luis Ruisinger on 05.10.23.
//

#ifndef C_REVERSE_PROXY_HASHMAP_H
#define C_REVERSE_PROXY_HASHMAP_H

struct Element
{
    char* value;
    struct Element* next;
};

struct Linkedlist
{
    char* key;
    struct Element* initial;
    struct Linkedlist* next;
};

struct Hashmap
{
    void   (*add) (char* key, char* value, struct Hashmap* hashmap);
    char** (*get) (char* key, struct Hashmap* hashmap);

    uint32_t size;
    struct Linkedlist** buckets;
};

struct Hashmap* hashmap_init(uint32_t size);

#endif //C_REVERSE_PROXY_HASHMAP_H
