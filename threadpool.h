#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <stddef.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct runnable {
  void (*function)(void *, size_t);
  void *arg;
  size_t argsz;
} runnable_t;

typedef struct thread_pool {

    pthread_mutex_t lock;
    pthread_cond_t workers;

    size_t orders_count;
    size_t first, orders_size;
    runnable_t *orders;

    int should_close;
    size_t num_threads;

} thread_pool_t;

int thread_pool_init(thread_pool_t *pool, size_t pool_size);

void thread_pool_destroy(thread_pool_t *pool);

int defer(thread_pool_t *pool, runnable_t runnable);

#endif
