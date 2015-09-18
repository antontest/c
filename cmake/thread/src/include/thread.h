#ifndef __THREAD_H__
#define __THREAD_H__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sys/select.h>
#include <sys/sysinfo.h>

typedef enum thread_status {
    THREAD_IDLE      = 0,
    THREAD_CREATING     ,
    THREAD_RUNNING      ,
    THREAD_STOPPED      ,
    THREAD_LOCK         ,
    THREAD_WAIT         ,
    THREAD_OVER         
} thread_status;

/**
 * @brief define function pointer
 * @function used when creating pthread
 */
typedef void* (*thread_handler)(void*);

/**
 * @brief callback of thread
 */
typedef struct thread_worker 
{
    thread_handler handler;
    void *arg;
} thread_worker_t;

/**
 * @brief thread info package
 */
typedef struct thread 
{
    /**
     * control parameters
     */
    int     active;
    int     run;
    int     repeat;
    int     delete;
    int     done;
    int     hold;
    long    create_time;
    long    delete_time;

    /**
     * info of thread
     */
    int           id;
    const char    *name;
    thread_status st;

    /**
     * thread
     */
    pthread_t           pid;
    pthread_mutex_t     lock;
    pthread_cond_t      ready;
    thread_worker_t     worker;
    thread_worker_t     free;

    struct thread  *next;
} thread_t;

/**
 * @brief info of thread queue
 */
typedef struct thread_pool {
    int thread_total_cnt;
    int thread_max_cnt;
    int thread_idle_cnt;
    int thread_mini_cnt;
    int active;

    pthread_t           pid;
    pthread_mutex_t     lock;
    pthread_cond_t      ready;
    struct thread  *head;
    struct thread  *tail;
} thread_pool_t;

/**
 * @brief thread configure
 */
typedef struct thread_cfg {
    char *name;
    int  repeat;
    int  run;
    
    thread_worker_t worker;
    thread_worker_t free;
} thread_cfg_t;

/**
 * global variable
 */ 
struct thread_pool *qthread;
struct thread_pool *pool;

/**
 * @brief start a pthread
 *
 * @param pt  [out] pthread ID
 * @param cr  [in] pthread function
 * @param arg [in] parameter you want to transfer into pthread function
 *
 * @return 0, if succ; -1, if failed.
 */
int thread_start(thread_t *impl, thread_handler handler, void *arg, int run, int repeat);

/**
 * @brief pthread exec another function
 *
 * @param impl [in]
 * @param pr   [in] callback
 * @param arg  [in]
 */
void thread_exec(thread_t *impl, thread_handler handler, void *arg);

/**
 * @brief exec function when pthread over
 *
 * @param impl [in]
 * @param pr   [in] callback
 * @param arg  [in]
 */
void thread_on_exit(thread_t *impl, thread_handler handler, void *arg);

/**
 * @brief let pthread run
 *
 * @param impl [in] 
 */
void thread_run(thread_t *impl);

/**
 * @brief let pthread stop
 *
 * @param impl [in] 
 */
void thread_stop(thread_t *impl);

/**
 * @brief lock a thread
 * 
 * @param mtx [in] mutex
 *
 * @return 0, if succ; -1, if falied.
 */
int thread_lock(thread_t *impl);

/**
 * @brief lock a thread
 * 
 * @param mtx [in] mutex
 *
 * @return 0, if succ; -1, if falied.
 */
int thread_trylock(thread_t *impl);

/**
 * @brief unlock a thread
 * 
 * @param mtx [in] mutex
 *
 * @return 0, if succ; -1, if falied.
 */
int thread_unlock(thread_t *impl);

/**
 * @brief destroy a lock of a thread
 * 
 * @param mtx [in] mutex
 *
 * @return 0, if succ; -1, if falied.
 */
int thread_delete(thread_t *impl);

/**
 * @brief wait pthread over 
 *
 * @param impl [in]
 */
void thread_wait_over(thread_t *impl);

/**
 * @brief wait pthread over 
 *
 * @param impl [in]
 */
void thread_time_wait_over(thread_t *impl, int tm_ms);

/**
 * @brief pthread_create 
 *
 * @param name    [in] name of thread
 * @param handler [in] callback
 * @param arg     [in] arg
 * @param run     [in] whether run at the time of creating
 * @param repeat    [in] whether run cycly
 *
 * @return 0, if succ; -1, if failed 
 */
int thread_create(const char *name, thread_handler handler, void *arg, int run, int repeat);

/**
 * @brief pthread_start 
 *
 * @param cfg [in] thread configure
 *
 * @return pthread_id, if succ; -1, if failed
 */
int pthread_start(struct thread_cfg *cfg);

/**
 * @brief get_thread 
 *
 * @param thread_idi [in] thread id
 *
 * @return thread impl, if succ; NULL, if failed
 */
struct thread * get_pthread(int thread_id);

/**
 * @brief pthread_run 
 *
 * @param thread_id [in] thread id
 */
void pthread_run(int thread_id);

/**
 * @brief pthread_stop 
 *
 * @param thread_id [in] thread id
 */
void pthread_stop(int thread_id);

/**
 * @brief pthread_delete
 *
 * @param thread_id [in] thread id
 */
void pthread_delete(int thread_id);

/**
 * @brief pthread_run 
 *
 * @param thread_id [in] thread id
 */
void pthread_hold(int thread_id);

/**
 * @brief pthread_unhold 
 *
 * @param thread_id [in] thread id
 */
void pthread_unhold(int thread_id);

/**
 * @brief pthread_lock 
 *
 * @param thread_id [in] thread id
 */
void pthread_lock(int thread_id);

/**
 * @brief pthread_unlock 
 *
 * @param thread_id [in] thread id
 */
void pthread_unlock(int thread_id);

/**
 * @brief pthread_wait 
 *
 * @param thread_id [in] thread id
 */
void pthread_wait(int thread_id);

/**
 * @brief pthread_unwait 
 *
 * @param thread_id [in] thread id
 */
void pthread_unwait(int thread_id);

/**
 * @brief pthread_info 
 */
void pthread_info();

/**
 * @brief pthread_pool_init 
 *
 * @param max_cnt  [in] mac count of thread in pthread pool
 * @param init_cnt [in] init count of thread in pthread pool
 *
 * @return 0, if succ; -1, if failed
 */
int pthread_pool_init(int max_cnt, int mini_cnt, int init_cnt);

/**
 * @brief get_idle_thread 
 *
 * @return thread, if succ; NULL, if failed
 */
struct thread * get_idle_thread();

/**
 * @brief pthread_pool_add 
 *
 * @param handler [in] pthread callback
 * @param arg     [in]
 *
 * @return 0, if succ; -1, if failed
 */
int pthread_pool_add(thread_handler handler, void *arg);

#endif
