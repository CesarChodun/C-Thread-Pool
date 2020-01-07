
#include "threadpool.h"
#include "future.h"

#define POOL_SIZE 3

void *fun(void* v, size_t argsz, size_t *ret_size) {
    int* ret = (int *) v;

    ret_size[0] = sizeof(int) * 2;
    int *out = (int*) malloc(sizeof(*ret_size));
    out[0] = ret[0] * (ret[1] + 1);
    out[1] = ret[1] + 1;

    return out;
}

int main() {

    int n;
    scanf("%d", &n);

    if (n == 0 || n == 1){
        printf("%d", 1);
        return 0;
    }

    thread_pool_t *pool = (thread_pool_t *) malloc(sizeof(thread_pool_t));
    thread_pool_init(pool, POOL_SIZE);

    callable_t cal;
    cal.function = &fun;
    cal.argsz = sizeof(int) * 2;
    cal.arg = malloc(cal.argsz);
    int* tmp = (int*) cal.arg;
    tmp[0] = 1;
    tmp[1] = 1;

    future_t* futures = (future_t*) malloc(sizeof(future_t) * (n + 1));

    async(pool, &futures[0], cal);

    for (int i = 0; i < n - 2; i++) {
        map(pool, &futures[i + 1], &futures[i], &fun);
    }

    void *ret = await(&futures[n - 2]);

    int *ret_vals = (int*)ret;
    int out = ret_vals[0];

    thread_pool_destroy(pool);
    free((void*) futures);
    free(pool);

    printf("%d\n", out);

    return 0;
}
