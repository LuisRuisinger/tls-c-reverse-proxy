//
// Created by Luis Ruisinger on 10.10.23.
//

#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <strings.h>

#include "run.h"
#include "tpool.h"
#include "pendingpool.h"

int32_t pool_add(struct Request_pool* pool, HTTP_Wrapper_struct* wrapper, struct Client* client, struct Server* server)
{
    if (pool == NULL)
        return EXIT_FAILURE;

    Pending_request* request = malloc(sizeof(Pending_request));
    assert(request != NULL);

    request->wrapper  = wrapper;
    request->client   = client;
    request->upstream = server;

    struct Linkedlist_pool* element = malloc(sizeof(struct Linkedlist_pool));
    assert(element != NULL);

    element->request = request;
    element->next    = NULL;

    pthread_mutex_lock(&(pool->work_mutex));

    FD_SET(server->socket, pool->pending);
    if (pool->first == NULL)
    {
        pool->first = element;
        pthread_cond_broadcast(&(pool->work_con));
    }
    else {
        pool->last->next = element;
        pool->last = element;
    }

    pthread_mutex_unlock(&(pool->work_mutex));
    return EXIT_SUCCESS;
}

_Noreturn void pool_update(struct Request_pool* pool)
{
    struct timeval timeout = {TIMEOUT, 0};
    while(1)
    {
        pthread_mutex_lock(&(pool->work_mutex));
        if (pool->first == NULL)
            pthread_cond_wait(&(pool->work_con), &(pool->work_mutex));
        else {
            int max_fd = 0;
            struct Linkedlist_pool* cur = pool->first;

            while (cur != NULL)
            {
                int cur_fd = cur->request->upstream->socket;
                if (cur_fd > max_fd)
                    max_fd = cur_fd;
                cur = cur->next;
            }

            fd_set* cpy = NULL;
            FD_ZERO(cpy);
            FD_COPY(pool->pending, cpy);

            if (select(max_fd + 1, cpy, NULL, NULL, &timeout) > 0)
            {
                struct Linkedlist_pool* bef = NULL;
                cur = pool->first;

                while (cur != NULL)
                {
                    if (FD_ISSET(cur->request->upstream->socket, cpy))
                    {
                        if (bef == NULL)
                            pool->first = cur->next;
                        FD_CLR(cur->request->upstream->socket, pool->pending);

                        Request* args = malloc(sizeof(Request));
                        assert(args != NULL);

                        args->client   = cur->request->client;
                        args->upstream = cur->request->upstream;

                        //
                        // still storing wrapper in case later needed ?
                        //

                        free(cur->request->wrapper);
                        free(cur->request);

                        tpool_add_work(pool->tpool, NULL, args); // change later to responsible read function
                    }
                    else {
                        bef = cur;
                    }
                    cur = cur->next;
                }
            }
            pthread_mutex_unlock(&(pool->work_mutex));
        }
    }
}

struct Request_pool* pool_init()
{
    struct Request_pool *pool = malloc(sizeof(struct Request_pool));
    assert(pool != NULL);

    pool->add = pool_add;
    pool->update = pool_update;

    pthread_mutex_init(&(pool->work_mutex), NULL);
    pthread_cond_init(&(pool->work_con), NULL);

    pool->first = NULL;
    pool->last = NULL;

    pool->tpool = tpool_create(sysconf(_SC_NPROCESSORS_CONF) + 1);
    FD_ZERO(pool->pending);

    return pool;
}