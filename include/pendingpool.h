//
// Created by Luis Ruisinger on 10.10.23.
//

#ifndef C_REVERSE_PROXY_PENDINGPOOL_H
#define C_REVERSE_PROXY_PENDINGPOOL_H

#include <pthread.h>
#include <sys/select.h>

#include "fieldparser.h"

typedef struct
{
    HTTP_Wrapper_struct* wrapper;
    struct Client* client;
    struct Server* upstream;
} Pending_request;

struct Linkedlist_pool
{
    Pending_request* request;
    struct Linkedlist_pool* next;
};

struct Request_pool
{
    struct Linkedlist_pool* first;
    struct Linkedlist_pool* last;
    fd_set* pending;

    pthread_cond_t work_con;
    pthread_mutex_t work_mutex;
    tpool_t* tpool;

    int32_t (*add)    (struct Request_pool* pool, HTTP_Wrapper_struct*, struct Client*, struct Server*);
    void    (*update) (struct Request_pool* pool);
};

struct Request_pool* pool_init(void);

#endif //C_REVERSE_PROXY_PENDINGPOOL_H
