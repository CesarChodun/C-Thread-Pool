#include "threadpool.h"

#define false 0
#define true 1

void worker(thread_pool_t *pool) {
    int err;
    if ((err = pthread_mutex_lock(&pool->lock) != 0))
        syserr(err, "mutex_lock");

    while (pool->should_close == false) {
        while (pool->order == NULL && pool->should_close == false)
            pthread_cond_wait(&pool->workers, &pool->lock);
        if (pool->should_close == true)
            break;

        //TODO: save the 'runnable' function

        pool->order = NULL;
        pthread_cond_signal(&pool->manager);

        if ((err = pthread_mutex_unlock(&pool->lock) != 0))
            syserr(err, "mutex_unlock");

        //TODO: run the 'runnable' function

        if ((err = pthread_mutex_lock(&pool->lock) != 0))
            syserr(err, "mutex_lock");
    }

    if ((err = pthread_mutex_unlock(&pool->lock) != 0))
        syserr(err, "mutex_unlock");
}

int thread_pool_init(thread_pool_t *pool, size_t num_threads) {

    int err;
    if ((err = pthread_mutex_init(&pool->lock, 0) != 0))
        return err;

    if ((err = pthread_cond_init(&pool->manager, 0)) != 0)
        return err;

    if ((err = pthread_cond_init(&pool->workers, 0)) != 0)
        return err;

    pool->should_close = false;
    pool->order = NULL;
    pool->num_threads = num_threads;

    pthread_t th[num_threads];
    pthread_attr_t attr;

    if ((err = pthread_attr_init(&attr)) != 0)
        return err;

    if ((err = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED)) != 0)
      return err;

    for (int i = 0; i < num_threads; i++) {
        if ((err = pthread_create(th[i], &attr, &worker, pool) != 0))
            return err;
    }

    return 0;
}

void thread_pool_destroy(struct thread_pool *pool) {
    int err;
    if ((err = pthread_mutex_lock(&pool->lock) != 0))
        syserr(err, "mutex_lock");

    pool->should_close = true;

    for (int i = 0; i < pool->num_threads; i++)
        pthread_cond_signal(&pool->workers);

    if ((err = pthread_mutex_unlock(&pool->lock) != 0))
        syserr(err, "mutex_unlock");
}

int defer(struct thread_pool *pool, runnable_t runnable) {
    int err;
    if ((err = pthread_mutex_lock(&pool->lock) != 0))
        return err;

    while (pool->order != NULL)
        pthread_cond_wait(&pool->manager, &pool->lock);

    pool->order = &runnable;
    pthread_cond_signal(&pool->workers);

    if ((err = pthread_mutex_unlock(&pool->lock) != 0))
        return err;

    return 0;
}
