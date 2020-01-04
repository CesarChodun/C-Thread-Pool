
#include "threadpool.h"
#include <unistd.h>

#define POOL_SIZE 4

pthread_cond_t master;
pthread_mutex_t lock;
int done;
int *mat;
int d;

void fun(void* v, size_t argsz) {
    int *data = (int*)v;
    int i = data[0];
    int val = data[1];
    int t = data[2];

    sleep(t);

    pthread_mutex_lock(&lock);
    //Get mutex
    mat[i] = val;
    done++;

    if (done == d)
        pthread_cond_signal(&master);

    pthread_mutex_unlock(&lock);
    // Release mutex
    // Maybe wake up master
}

int main() {

    done = 0;

    int err;
    if ((err = pthread_mutex_init(&lock, 0) != 0))
        return err;
    if ((err = pthread_cond_init(&master, 0)) != 0)
        return err;
    pthread_mutex_lock(&lock);

    thread_pool_t pool;
    thread_pool_init(&pool, POOL_SIZE);

    int n, m;
    scanf("%d\n%d", &n, &m);

    d = n * m;
    mat = (int*) malloc(sizeof(int) * d);
    runnable_t *runs = (runnable_t *) malloc(sizeof(runnable_t) * d);
    for (int i = 0; i < d; i++) {
        int v, t;
        scanf("%d %d", &v, &t);

        runs[i].function = &fun;
        runs[i].arg = malloc(sizeof(int) * 3);
        ((int*)runs[i].arg)[0] = i;
        ((int*)runs[i].arg)[1] = v;
        ((int*)runs[i].arg)[2] = t;
        runs[i].argsz = sizeof(int) * 3;

        defer(&pool, runs[i]);
    }

    while (done != d)
        pthread_cond_wait(&master, &lock);

    if ((err = pthread_mutex_unlock(&lock) != 0))
        return err;

    long long out = 0;
    for (int i = 0; i < d; i++) {
        out += mat[i];
        
        if ((i + 1) % m == 0) {
            printf("%lld \n", out);
            out = 0;
        }
    }

    thread_pool_destroy(&pool);

    for (int i = 0; i < d; i++)
        free(runs[i].arg);
    free((void*) runs);

    return 0;
}
