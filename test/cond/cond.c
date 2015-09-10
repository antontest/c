#include <stdio.h>
#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>

pthread_mutex_t mtx;
pthread_cond_t cnd;

void* fun(void *arg)
{
    pthread_mutex_lock(&mtx);
    pthread_cond_wait(&cnd, &mtx);
    printf("pid: %u, fun %s\n", (unsigned int)pthread_self(), (char *)arg);
    pthread_mutex_unlock(&mtx);

    return NULL;
}

int main()
{
    pthread_t pid;
    pthread_t pid1;

    pthread_create(&pid, NULL, fun, "1");
    pthread_create(&pid1, NULL, fun, "2");
    pthread_mutex_init(&mtx, NULL);
    pthread_cond_init(&cnd, NULL);

    sleep(2);
    //pthread_cond_signal(&cnd);
    //sleep(3);
    //pthread_cond_signal(&cnd);
    pthread_cond_broadcast(&cnd);

    pthread_join(pid, NULL);
    pthread_join(pid1, NULL);

    return 0;
}
