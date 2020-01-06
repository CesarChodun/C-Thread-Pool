#include "future.h"

typedef void *(*function_t)(void *);

typedef struct data {
    future_t *future;
    callable_t callable;
} data_t;

void calculate(void *v, size_t argsz) {
    data_t info = ((data_t *)v)[0];

    size_t ret_size;
    info.future->ret = info.callable.function(info.callable.arg, info.callable.argsz, &ret_size);
    sem_post(&info.future->work_done);
}

runnable_t newRunnable(future_t *future, callable_t callable) {

    data_t info;
    info.future = future;
    info.callable = callable;

    runnable_t out;
    out.function = &calculate;
    out.argsz = sizeof(data_t);
    out.arg = (data_t *)malloc(out.argsz);

    ((data_t *)out.arg)[0] = info;

    return out;
}

int async(thread_pool_t *pool, future_t *future, callable_t callable) {

    sem_init(&future->work_done, 0, 0);
    runnable_t run = newRunnable(future, callable);
    return defer(pool, run);
}

int map(thread_pool_t *pool, future_t *future, future_t *from,
        void *(*function)(void *, size_t, size_t *)) {


    return 0;
}

void *await(future_t *future) {
    sem_wait(&future->work_done);
    sem_destroy(&future->work_done);
    return future->ret;
}
