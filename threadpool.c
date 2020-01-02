#include "threadpool.h"
#include <pthread.h>

#define false 0
#define true 1

void addRunnable(thread_pool_t *pool, runnable_t runnable) {
    pool->orders_count++;

    if (pool->orders_count > pool->orders_size) {
        size_t old_size = pool->orders_size;
        pool->orders_size *= 2;

        pool->orders = (runnable_t *) realloc(pool->orders,
            pool->orders_size * sizeof(runnable_t));

        for (size_t i = 0; i < pool->first; i++)
            pool->orders[old_size + i] = pool->orders[i];
    }

    size_t npos = (pool->orders_count + pool->first - 1) % pool->orders_size;
    pool->orders[npos] = runnable;
}

// queue cannot be empty
runnable_t popRunnable(thread_pool_t *pool) {
    runnable_t out = pool->orders[pool->first];
    pool->first = (pool->first + 1) % pool->orders_size;
    pool->orders_count--;
    return out;
}

void *worker(void *args) {
    thread_pool_t *pool = (thread_pool_t*) args;
    pthread_mutex_lock(&pool->lock);

    while (pool->should_close == false) {
        while (pool->orders_count == 0 && pool->should_close == false)
            pthread_cond_wait(&pool->workers, &pool->lock);
        if (pool->should_close == true)
            break;

        runnable_t run = popRunnable(pool);

        pthread_mutex_unlock(&pool->lock);
        run.function(run.arg, run.argsz);
        pthread_mutex_lock(&pool->lock);
    }

    pthread_mutex_unlock(&pool->lock);
}

int thread_pool_init(thread_pool_t *pool, size_t num_threads) {

    int err;
    if ((err = pthread_mutex_init(&pool->lock, 0) != 0))
        return err;

    if ((err = pthread_cond_init(&pool->workers, 0)) != 0)
        return err;

    pool->should_close = false;
    pool->num_threads = num_threads;

    pool->orders_count = 0;
    pool->first = 0;
    pool->orders_size = 4;
    pool->orders = (runnable_t *) malloc(4 * sizeof(runnable_t));


    pthread_t th[num_threads];
    pthread_attr_t attr;

    if ((err = pthread_attr_init(&attr)) != 0)
        return err;

    if ((err = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED)) != 0)
      return err;

    for (int i = 0; i < num_threads; i++) {
        if ((err = pthread_create(&th[i], &attr, worker, pool) != 0))
            return err;
    }

    return 0;
}

void thread_pool_destroy(struct thread_pool *pool) {
    pthread_mutex_lock(&pool->lock);

    pool->should_close = true;
    pool->orders_count = 0;
    free(pool->orders);

    for (size_t i = 0; i < pool->num_threads; i++)
        pthread_cond_signal(&pool->workers);

    pthread_mutex_unlock(&pool->lock);
}

int defer(struct thread_pool *pool, runnable_t runnable) {
    int err;
    if ((err = pthread_mutex_lock(&pool->lock) != 0))
        return err;

    addRunnable(pool, runnable);
    pthread_cond_signal(&pool->workers);

    if ((err = pthread_mutex_unlock(&pool->lock) != 0))
        return err;

    return 0;
}
