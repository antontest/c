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
 * @brief Binary semaphore
 */
typedef struct bsem {
    pthread_mutex_t mutex;
    pthread_cond_t  cond;
    int v;
} bsem;

/**
 * @brief Job
 */
typedef struct job {
    struct job *prev;           /* pointer to previous job */
    void* (*function)(void *arg);/* function pointer */
    void* arg;                  /* function's argument */
} job;

/**
 * @brief Job queue
 */
typedef struct jobqueue {
    pthread_mutex_t rwlock;  /* used for queue r/w access */
    job *front;              /* pointer to front of queue */
    job *rear;               /* pointer to rear of queue */
    bsem *has_jobs;          /* flag as binary semaphore */
    int len;                 /* number of jobs in queue */
} jobqueue;

typedef struct thpool thpool;
typedef struct thpool *threadpool;

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
    pthread_t        pid;    /* pthread id, gived by pthread_create*/
    struct job       worker; /* pthread callback function, pthread main task */
    struct job       clean;  /* happened when thread be deleted or canceled*/
    pthread_mutex_t  lock; /* pthread mutex */
    pthread_cond_t   ready;/* pthread cond */

    /**
     * access to thpool 
     */
   struct thpool *thpool_p;
} thread;


/**
 * @brief Thread pool
 */
typedef struct thpool {
    thread **threads;                /* pointer to thresds */
    volatile int num_threads_alive;  /* threads currently alive */
    volatile int num_threads_working;/* threads currently working */
    pthread_mutex_t thcount_lock;    /* used for thread count etc */
    jobqueue *jobqueue_p;            /* pointer to the job queue */
} thpool_;

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



/******************************************************
 *************** Thread Pool Function *****************
 ******************************************************/
/* =================== SYNCHRONISATION =================== */
/**
 * @brief init semaphore to 1 or 0
 *
 * @param pbsem [in] binary semaphore
 * @param value [in] take only 1 or 0
 */
void bsem_init(bsem *pbsem, int value);

/**
 * @brief reset semaphore to 0
 *
 * @param pbsem [in] semaphore
 */
void bsem_reset(bsem *pbsem);

/**
 * @brief post to at least one thread
 *
 * @param pbsem
 */
void bsem_post(bsem *pbsem);

/**
 * @brief post all threads
 *
 * @param pbsem
 */
void bsem_post_all(bsem *pbsem);

/**
 * @brief wait on semaphore until semaphore has value 0
 *
 * @param pbsem
 */
void bsem_wait(bsem *pbsem);


/* ===================== THREAD POOL ===================== */
/**
 * @brief initialise thread pool
 *
 * @param num_threads
 *
 * @return 
 */
struct thpool * thpool_init(int num_threads);

/**
 * @brief add work to the thread pool
 *
 * @param thpool_p
 * @param function
 * @param arg
 *
 * @return 
 */
int thpool_add_work(thpool *thpool_p, void *(*function)(void*), void *arg);

/**
 * @brief wait until all jobs have finished
 *
 * @param thpool_p
 */
void thpool_wait(thpool *thpool_p);

/**
 * @brief destroy the thread pool
 *
 * @param thpool_p
 */
void thpool_destroy(thpool *thpool_p);

/**
 * @brief pause all threads in thread pool
 *
 * @param thpool_p
 */
void thpool_pause(thpool *thpool_p);

/**
 * @brief resume all threads in thread poo;
 *
 * @param thpool_p
 */
void thpool_resume(thpool *thpool_p);

#endif
