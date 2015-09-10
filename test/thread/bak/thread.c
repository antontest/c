#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <sys/errno.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <getopt.h>
#include <time.h>

/**
 * @brief define function pointer
 * @function used when creating pthread
 */
typedef void* (*pthread_runtine)(void*);

/**
 * @brief callback function of pthread
 */
typedef struct pthread_worker {
    pthread_runtine runtine;
    void *arg;
} pthread_worker_t;

typedef struct thread_impl {
    int active;
    int run;
    int delete;
    int done;
    pthread_t pid;
    pthread_worker_t worker;
    pthread_mutex_t lock;
    pthread_cond_t ready;
} thread_t;

/*********************************************************
 **************    Function Declaration    ***************
 *********************************************************/
int pthread_start(thread_t *pt, pthread_runtine runtine, void *arg, int run);
void pthread_exec(thread_t *pt, pthread_runtine runtine, void *arg);
void pthread_run(thread_t *pt);
void pthread_wait_over(thread_t *pt);
int pthread_delete(thread_t *pt);
int pthread_lock(thread_t *pt);
int pthread_trylock(thread_t *pt);
int pthread_unlock(thread_t *pt);

void* echo(void *arg)
{
    while (1)
    {
        //select(10,NULL,NULL,NULL,NULL);
        printf("hello: %s\n", (char *)arg);
        sleep(1);
        pthread_testcancel();
        //sleep(2);
    }
    
    printf("test cancel failed\n");
    return NULL;
}

void *test(void *arg)
{
    printf("test %s\n", (char *)arg);

    return NULL;
}

/*********************************************************
 ******************    Main Function    ******************
 *********************************************************/
int main(int agrc, char *agrv[])
{
    int rt = 0;
    thread_t pit;

    pthread_start(&pit, echo, "1", 1);
    sleep(2);
    pthread_cancel(pit.pid);
    //pthread_exec(&pit, test, "2");
    sleep(2);
    //pthread_cancel(pit.pid);
    //pthread_delete(&pit);
    pthread_wait_over(&pit);

    return rt;
}

/**
 * @brief pthread runtine
 *
 * @param arg [in] struct pthread_impl pointer
 *
 * @return 
 */
void *thread_runtine(void *arg)
{
    thread_t *impl = (thread_t *)arg;
    pthread_worker_t *worker = &impl->worker;

    while (impl->active) 
    {
        worker = &impl->worker;
        if (!impl->run) continue;
        if (impl->delete) break;

        if (worker->runtine != NULL)
        {
            worker->runtine(worker->arg);

            worker->runtine = NULL;
            worker->arg = NULL;
            impl->done = 1;
        }
    }

    impl->active = 0;
    impl->delete = 1;
    impl->done = 1;
    impl->run = 0;

    pthread_mutex_destroy(&impl->lock);
    pthread_cond_destroy(&impl->ready);

    return NULL;
}

/**
 * @brief start a pthread
 *
 * @param pt  [out] pthread ID
 * @param cr  [in] pthread function
 * @param arg [in] parameter you want to transfer into pthread function
 *
 * @return 0, if succ; -1, if failed.
 */
int pthread_start(thread_t *impl, pthread_runtine runtine, void *arg, int run)
{
    if (impl == NULL || !impl->active) return -1;

    impl->worker.runtine = runtine;
    impl->worker.arg = arg;

    impl->run = run;
    impl->done = 0;
    impl->delete = 0;
    impl->active = 1;

    pthread_mutex_init(&impl->lock, NULL);
    pthread_cond_init(&impl->ready, NULL);

    return pthread_create(&impl->pid, NULL, thread_runtine, impl);
}

/**
 * @brief let pthread run
 *
 * @param impl [in] 
 */
void pthread_run(thread_t *impl)
{
    if (impl == NULL || !impl->active) return;

    impl->run = 1;

    return;
}

/**
 * @brief pthread exec another function
 *
 * @param impl [in]
 * @param pr   [in] callback
 * @param arg  [in]
 */
void pthread_exec(thread_t *impl, pthread_runtine runtine, void *arg)
{
    if (impl == NULL || !impl->active) return;

    impl->run = 0;
    impl->worker.runtine = runtine;
    impl->worker.arg = arg;

    impl->run = 1;
    impl->done = 0;

    return;
}

/**
 * @brief lock a thread
 * 
 * @param mtx [in] mutex
 *
 * @return 0, if succ; -1, if falied.
 */
int pthread_lock(thread_t *impl)
{
    if (impl == NULL || impl->active) return -1;

    return pthread_mutex_lock(&impl->lock);
}

/**
 * @brief lock a thread
 * 
 * @param mtx [in] mutex
 *
 * @return 0, if succ; -1, if falied.
 */
int pthread_trylock(thread_t *impl)
{
    if (impl == NULL && !impl->active) return -1;

    return pthread_mutex_trylock(&impl->lock);
}

/**
 * @brief unlock a thread
 * 
 * @param mtx [in] mutex
 *
 * @return 0, if succ; -1, if falied.
 */
int pthread_unlock(thread_t *impl)
{
    if (impl == NULL && !impl->active) return -1;

    return pthread_mutex_unlock(&impl->lock);
}

/**
 * @brief destroy a lock of a thread
 * 
 * @param mtx [in] mutex
 *
 * @return 0, if succ; -1, if falied.
 */
int pthread_delete(thread_t *impl)
{
    int wait_tm = 10 * 100;
    if (impl == NULL || !impl->active) return -1;

    if (impl->run)
    {
        while(wait_tm-- > 0)
        {
            if (impl->done) break;
            else usleep(10);
        }
    }

    impl->delete = 1;
    sleep(1);

    if (!impl->done)
    {
        pthread_cancel(impl->pid);
        pthread_mutex_destroy(&impl->lock);
        pthread_cond_destroy(&impl->ready);
    }

    return 0;
}

/**
 * @brief wait pthread over 
 *
 * @param impl [in]
 */
void pthread_wait_over(thread_t *impl)
{
    if (impl == NULL || !impl->active) return;

    pthread_join(impl->pid, NULL);

    return;
}

void pthread_wait_done(thread_t *impl)
{
    if (impl == NULL || !impl->active) return;

    return;
}
