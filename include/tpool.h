//
// Created by Luis Ruisinger on 05.10.23.
//

#ifndef C_REVERSE_PROXY_TPOOL_H
#define C_REVERSE_PROXY_TPOOL_H

struct tpool;
typedef struct tpool tpool_t;

typedef void (*thread_func_t) (void *arg);

tpool_t *tpool_create(size_t num);
void tpool_destroy(tpool_t *tm);
bool tpool_add_work(tpool_t *tm, thread_func_t func, void *arg);
void tpool_wait(tpool_t *tm);

#endif //C_REVERSE_PROXY_TPOOL_H
