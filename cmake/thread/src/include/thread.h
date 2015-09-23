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
#include <semaphore.h>

#define DFT_THREAD_TIMEOUT 10

typedef enum thread_status {
    THREAD_IDLE      = 0,  /* at thread init and execute task over */
    THREAD_CREATING     ,  /* at creating thread */
    THREAD_RUNNING      ,  /* at getting into thread runtine, but before execute */
    THREAD_BUSY         ,  /* at executing thread task */
    THREAD_STOPPED      ,  /* at stopped */
    THREAD_LOCK         ,  /* at locked */
    THREAD_WAIT         ,  /* at waiting other thread */
    THREAD_OVER            /* at thread over */
} thread_status;

/**
 * @brief define function pointer
 * @function used when creating pthread
 */
typedef void* (*thread_handler)(void*);

/**
 * @brief define function pointer
 * @function used when thread canceled for cleaning up
 */
typedef void (*cleanup_handler)(void*);


/******************************************************
 ********************* Struct *************************
 ******************************************************/
/**
 * @brief callback of thread
 */
typedef struct thread_worker 
{
    void                 *arg;
    thread_handler       handler;
    struct thread_worker *next;
} thread_worker_t;

/**
 * @brief thread info package
 */
typedef struct thread 
{
    /**
     * control parameters
     */
    int     active;      /* active thread */
    int     run;         /* control thread run or pause */
    int     repeat;      /* thread can repeat any times */
    int     delete;      /* destroy this thread */
    int     hold;        /* keep this thread in memory */
    long    create_time; /* create time */
    long    delete_time; /* delete time */
    thread_status state; /* thead state */

    /**
     * info of thread
     */
    int         id;      /* everyone has a diffrent id */
    const char  *name;   /* can give thread a name */

    /**
     * thread
     */
    pthread_t           pid;    /* pthread id, gived by pthread_create*/
    thread_worker_t     worker; /* pthread callback function, pthread main task */
    thread_worker_t     clean;  /* happened when thread be deleted or canceled*/
    pthread_mutex_t     lock; /* pthread mutex */
    pthread_cond_t      ready;/* pthread cond */

    /**
     * pointer to next thread
     */
    struct thread  *next;
} thread_t;

/**
 * @brief task pool
 */
typedef struct task_pool {
    struct thread   *head;
    struct thread   *tail;
} task_pool_t;

/**
 * @brief info of thread queue
 */
typedef struct thread_pool {
    int active;                  /* active thread pool runtine */
    int run;                     /* run thread pool runtine */
    int exit;                    /* run thread pool runtine */
    int state;                   /* thread pool state */

    /**
     * pool info
     */
    int free_time;               /* thread timeout when idle  */
    int thread_max_cnt;          /* max count of thread pool  */
    int thread_mini_cnt;         /* mini count of idle thread */
    int thread_total_cnt;        /* all threads count in pool */

    /**
     * thread control
     */
    pthread_t        pid;        /* pid of thread pool */
    pthread_mutex_t  lock;       /* pthread mutex */
    pthread_cond_t   ready;      /* pthread cond */
    sem_t            sem;        /* signal */
    
    /**
     * pool
     */
    struct task_pool task_pool;  /* thread task queue */
    struct task_pool idle_pool;  /* idle thread queue */
    struct task_pool run_pool;   /* trhead work queue */
} thread_pool_t;

/**
 * @brief thread configure
 */
typedef struct thread_cfg {
    char *name;  /* can give thread a name */
    int  repeat; /* can repeat thread task */
    int  run;    /* let thread start to execute task */
    
    thread_worker_t worker; /* thread callback */
    thread_worker_t clean;  /* executed when destroyed thread */
} thread_cfg_t;

/**
 * global variable
 */ 
struct thread_pool *qthread;
struct thread_pool *pool;




/******************************************************
 ************** Pthread Basic Function ****************
 ******************************************************/
/**
 * @brief pstart -- start a thread
 *
 * @param handler [in] callback
 * @param arg     [in] arg
 *
 * @return pid of thread, if succ; -1, if failed
 */
pthread_t pstart(thread_handler handler, void *arg);

/**
 * @brief pcreate 
 *
 * @param handler [in] callback
 * @param arg     [in] arg
 *
 * @return pid, if succ; -1, if failed
 */
pthread_t pcreate(thread_handler handler, void *arg);

/**
 * @brief lock -- lock thread
 *
 * @param mtx [in] pthread mutex
 */
void plock(pthread_mutex_t *mtx);

/**
 * @brief ptrylock -- lock thread
 *
 * @param mtx [in] pthread mutex
 */
void ptrylock(pthread_mutex_t *mtx);

/**
 * @brief unlock -- unlock thread
 *
 * @param mtx [in] pthread mutex
 */
void punlock(pthread_mutex_t *mtx);

/**
 * @brief wait -- wait another thread
 *
 * @param cond [in] pthread cond
 * @param mtx  [in] pthred mutex
 */
void pwait(pthread_cond_t *cond, pthread_mutex_t *mtx);

/**
 * @brief pcontinue -- let another thread go on
 *
 * @param cond [in] pthread cond
 */
void pcontinue(pthread_cond_t *cond);

/**
 * @brief wait pthread over 
 *
 * @param pthread [in]
 */
void pjoin(pthread_t pid);

/**
 * @brief pexit -- wait for thread over safely 
 *
 * @param rval [out] thread return value
 */
void pexit(void *rval);

/**
 * @brief pkill -- kill thread
 *
 * @param pid    [in] pid
 * @param signal [in] signal number
 *
 * @return 0, if thread exit and succ; EINVAL or ESRCH, if failed
 */
int pkill(pthread_t pid, int signal);

/**
 * @brief pcancel 
 *
 * @param pid [in] id
 *
 * @return 
 */
int pcancel(pthread_t pid);


/******************************************************
*************** Pthread Attribute Function ************
******************************************************/
/**
 * @brief enable_cancel 
 */
int enable_cancel();

/**
 * @brief enable_cancel 
 */
int disable_cancel();

/**
 * @brief set_cancel_asyn 
 */
int set_cancel_asyn();

/**
 * @brief set_cancel_asyn 
 */
int set_cancel_defe();

/**
 * @brief set_joinable -- set thread joinable
 *
 * @return 0, if succ; -1, if failed
 */
int set_joinable();

/**
 * @brief set_detach -- set thread detached
 *
 * @return 0, if succ; -1, if failed
 */
int set_detach();


/******************************************************
*************** Pthread Manage Function ****************
******************************************************/
/**
 * @brief start a pthread
 *
 * @param pt  [out] pthread ID
 * @param cr  [in] pthread function
 * @param arg [in] parameter you want to transfer into pthread function
 *
 * @return 0, if succ; -1, if failed.
 */
int pthread_start(thread_handler handler, void *arg, int run, int repeat);

/**
 * @brief pthread exec another function
 *
 * @param pthread [in]
 * @param pr   [in] callback
 * @param arg  [in]
 */
void pthread_exec(thread_t *pthread, thread_handler handler, void *arg);

/**
 * @brief exec function when pthread over
 *
 * @param pthread [in]
 * @param pr   [in] callback
 * @param arg  [in]
 */
void pthread_on_exit(thread_t *pthread, thread_handler handler, void *arg);

/**
 * @brief let pthread run
 *
 * @param pthread [in] 
 */
void pthread_run(thread_t *pthread);

/**
 * @brief let pthread stop
 *
 * @param pthread [in] 
 */
void pthread_stop(thread_t *pthread);

/**
 * @brief destroy a lock of a thread
 * 
 * @param mtx [in] mutex
 *
 * @return 0, if succ; -1, if falied.
 */
int pthread_delete(thread_t *pthread);

/**
 * @brief wait pthread over 
 *
 * @param pthread [in]
 */
void pthread_time_wait_over(thread_t *pthread, int tm_ms);

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
//int thread_create(const char *name, thread_handler handler, void *arg, int run, int repeat);

/**
 * @brief pthread_start 
 *
 * @param cfg [in] thread configure
 *
 * @return pthread_id, if succ; -1, if failed
 */
//int pthread_start(struct thread_cfg *cfg);

/**
 * @brief get_thread 
 *
 * @param thread_idi [in] thread id
 *
 * @return thread pthread, if succ; NULL, if failed
 */
struct thread * get_pthread(int thread_id);

/**
 * @brief pthread_run 
 *
 * @param thread_id [in] thread id
 */
void thread_run(int thread_id);

/**
 * @brief pthread_stop 
 *
 * @param thread_id [in] thread id
 */
void thread_stop(int thread_id);

/**
 * @brief pthread_delete
 *
 * @param thread_id [in] thread id
 */
void thread_delete(int thread_id);

/**
 * @brief pthread_run 
 *
 * @param thread_id [in] thread id
 */
void thread_hold(int thread_id);

/**
 * @brief pthread_unhold 
 *
 * @param thread_id [in] thread id
 */
void thread_unhold(int thread_id);

/**
 * @brief pthread_lock 
 *
 * @param thread_id [in] thread id
 */
void thread_lock(int thread_id);

/**
 * @brief pthread_unlock 
 *
 * @param thread_id [in] thread id
 */
void thread_unlock(int thread_id);

/**
 * @brief pthread_wait 
 *
 * @param thread_id [in] thread id
 */
void thread_wait(int thread_id);

/**
 * @brief pthread_unwait 
 *
 * @param thread_id [in] thread id
 */
void thread_unwait(int thread_id);

/**
 * @brief pthread_info 
 */
void thread_info(struct task_pool *task_pool);


/******************************************************
****************       Pthread Pool    ****************
*******************************************************/
/**
 * @brief get_pool_size 
 *
 * @param pool
 *
 * @return 
 */
int get_pool_size(struct task_pool *pool);

/**
 * @brief pthread_pool_init 
 *
 * @param max_cnt  [in] mac count of thread in pthread pool
 * @param mini_cnt [in] mini count of thread in pthread pool
 * @param init_cnt [in] init count of thread in pthread pool
 * @param tm       [in] timeout of thread in pthread pool
 *
 * @return 0, if succ; -1, if failed
 */
int pthread_pool_init(int max_cnt, int mini_cnt, int init_cnt, int tm);


/**
 * @brief pthread_pool_add 
 *
 * @param handler [in] pthread callback
 * @param arg     [in]
 *
 * @return 0, if succ; -1, if failed
 */
int pthread_pool_add(thread_handler handler, void *arg);

/**
 * @brief dequeue_pool 
 *
 * @param pool
 *
 * @return 
 */
struct thread * dequeue_pool(struct task_pool *pool);

#endif
