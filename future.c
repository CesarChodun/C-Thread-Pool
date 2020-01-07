#include "future.h"

typedef void *(*function_t)(void *);

typedef struct data {
    future_t *future;
    callable_t callable;
} data_t;

typedef struct mapped_data {
    future_t *future;
    future_t *from;
    void *(*function)(void *, size_t, size_t *);
} mapped_data_t;

void calculate(void *v, size_t argsz) {
    if (argsz < sizeof(data_t *))
        return;

    data_t info = ((data_t *)v)[0];

    info.future->ret = info.callable.function(
        info.callable.arg,
        info.callable.argsz,
        &info.future->ret_size);
    sem_post(&info.future->work_done);
}

void calculateMapped(void *v, size_t argsz) {
    if (argsz < sizeof(mapped_data_t *))
        return;

    mapped_data_t info = ((mapped_data_t *)v)[0];

    sem_wait(&info.from->work_done);
    info.future->ret = info.function(
        info.from->ret,
        info.from->ret_size,
        &info.future->ret_size);
    sem_post(&info.future->work_done);
}

runnable_t newRunnable(future_t *future, callable_t callable) {

    data_t info;
    info.future = future;
    info.callable = callable;

    runnable_t out;
    out.function = &calculate;
    out.argsz = sizeof(data_t);
    out.arg = malloc(out.argsz);

    ((data_t *)out.arg)[0] = info;

    return out;
}

runnable_t newMappedRunnable(future_t *future, future_t *from,
    void *(*function)(void *, size_t, size_t *)) {

    mapped_data_t info;
    info.future = future;
    info.from = from;
    info.function = function;

    runnable_t out;
    out.function = &calculateMapped;
    out.argsz = sizeof(mapped_data_t);
    out.arg = malloc(out.argsz);

    ((mapped_data_t *)out.arg)[0] = info;

    return out;
}

int async(thread_pool_t *pool, future_t *future, callable_t callable) {
    sem_init(&future->work_done, 0, 0);
    runnable_t run = newRunnable(future, callable);
    return defer(pool, run);
}

int map(thread_pool_t *pool, future_t *future, future_t *from,
        void *(*function)(void *, size_t, size_t *)) {
    sem_init(&future->work_done, 0, 0);
    runnable_t run = newMappedRunnable(future, from, function);
    return defer(pool, run);
}

void *await(future_t *future) {
    sem_wait(&future->work_done);
    sem_destroy(&future->work_done);
    return future->ret;
}
