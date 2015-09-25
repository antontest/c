#ifndef __THPOOL_H__
#define __THPOOL_H__
#include "bsem.h"
#include "thread.h"

typedef struct thpool thpool;
typedef struct thpool *threadpool;

/******************************************************
 ********************* Struct *************************
 ******************************************************/
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
 ************** Thread Pool Function ******************
 ******************************************************/

/******************************************************
 *************** Thread Pool Function *****************
 ******************************************************/
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
