#ifndef __THREAD_H__
#define __THREAD_H__
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

typedef struct pthread_worker 
{
    pthread_runtine runtine;
    void *arg;
} pthread_worker_t;

typedef struct thread_impl 
{
    int active;
    int run;
    int delete;
    int done;
    pthread_t pid;
    pthread_worker_t worker;
    pthread_mutex_t lock;
    pthread_cond_t ready;
} thread_t;

/**
 * @brief start a pthread
 *
 * @param pt  [out] pthread ID
 * @param cr  [in] pthread function
 * @param arg [in] parameter you want to transfer into pthread function
 *
 * @return 0, if succ; -1, if failed.
 */
int pthread_start(thread_t *impl, pthread_runtine runtine, void *arg, int run);

/**
 * @brief pthread exec another function
 *
 * @param impl [in]
 * @param pr   [in] callback
 * @param arg  [in]
 */
void pthread_exec(thread_t *impl, pthread_runtine runtine, void *arg);

/**
 * @brief let pthread run
 *
 * @param impl [in] 
 */
void pthread_run(thread_t *impl);

/**
 * @brief lock a thread
 * 
 * @param mtx [in] mutex
 *
 * @return 0, if succ; -1, if falied.
 */
int pthread_lock(thread_t *impl);

/**
 * @brief lock a thread
 * 
 * @param mtx [in] mutex
 *
 * @return 0, if succ; -1, if falied.
 */
int pthread_trylock(thread_t *impl);

/**
 * @brief unlock a thread
 * 
 * @param mtx [in] mutex
 *
 * @return 0, if succ; -1, if falied.
 */
int pthread_unlock(thread_t *impl);

/**
 * @brief destroy a lock of a thread
 * 
 * @param mtx [in] mutex
 *
 * @return 0, if succ; -1, if falied.
 */
int pthread_delete(thread_t *impl);

#endif
