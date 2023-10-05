//
// Created by Luis Ruisinger on 05.10.23.
//

#ifndef C_REVERSE_PROXY_HASHMAP_H
#define C_REVERSE_PROXY_HASHMAP_H

enum Type
{
    STATICFILE, PROTOCOL
};

struct Element
{
    char* value;
    struct Element* next;
};

struct Linkedlist
{
    char* key;
    enum Type type;
    struct Element* initial;
    struct Linkedlist* next;
};

struct Hashmap
{
    void   (*add)     (struct Hashmap* hashmap, char* key, enum Type type, char* value);
    void   (*add_all) (struct Hashmap* hashmap, char* key, enum Type type, char* f_value, ...);
    char** (*get)     (struct Hashmap* hashmap, char* key, enum Type type);

    uint32_t size;
    struct Linkedlist** buckets;
};

struct Hashmap* hashmap_init(uint32_t size);

#endif //C_REVERSE_PROXY_HASHMAP_H
