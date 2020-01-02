
#include "threadpool.c"

#define POOL_SIZE 4

int main() {

    thread_pool_t pool;
    thread_poool_init(&pool, POOL_SIZE);



    thread_pool_destroy(&pool);

    return 0;
}
