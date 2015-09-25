#include "thpool.h"

static volatile int threads_id = 1;
static volatile int threads_keepalive;
static volatile int threads_on_hold;

#ifdef THPOOL_DEBUG
#define THPOOL_DEBUG 1
#else
#define THPOOL_DEBUG 0
#endif

#define MAX_NANOSEC 999999999
#define CEIL(X) ((X - (int)X) > 0 ? (int)(X + 1) : (int)(X))


/******************************************************
 *************** Thread Pool Function *****************
 ******************************************************/
/* ====================== JOB QUEUE ====================== */
/**
 * @brief initialize queue
 *
 * @param thpool_p [in] pointer to thread pool
 *
 * @return 0, if succ; -1, if failed
 */
int jobqueue_init(thpool *thpool_p)
{
    thpool_p->jobqueue_p = (struct jobqueue *)malloc(sizeof(struct jobqueue));
    if (thpool_p->jobqueue_p == NULL) return -1;
    thpool_p->jobqueue_p->len = 0;
    thpool_p->jobqueue_p->front = NULL;
    thpool_p->jobqueue_p->rear = NULL;

    thpool_p->jobqueue_p->has_jobs = (struct bsem *)malloc(sizeof(struct bsem));
    if (thpool_p->jobqueue_p->has_jobs == NULL) return -1;

    pthread_mutex_init(&thpool_p->jobqueue_p->rwlock, NULL);
    bsem_init(thpool_p->jobqueue_p->has_jobs, 0);
    
    return 0;
}

/**
 * @brief add (allocateed) job to queue
 *
 * @param thpool_p [in] pointer ot thread pool
 * @param newjob   [in] new job
 */
void jobqueue_push(thpool *thpool_p, struct job *newjob)
{
    newjob->prev = NULL;
    switch (thpool_p->jobqueue_p->len) {
        case 0: /* if no jobs in queue */
            thpool_p->jobqueue_p->front = newjob;
            thpool_p->jobqueue_p->rear = newjob;
            break;
        default: /* if jobs in queue */
            thpool_p->jobqueue_p->rear->prev = newjob;
            thpool_p->jobqueue_p->rear = newjob;
            break;

    }
    thpool_p->jobqueue_p->len++;

    bsem_post(thpool_p->jobqueue_p->has_jobs);
}

/**
 * @brief get first job from queue (remove it from queue)
 *
 * @param thpool_p [in] pointer to thread pool
 *
 * @return job
 */
struct job * jobqueue_pull(thpool *thpool_p)
{
    job *job_p;
    job_p = thpool_p->jobqueue_p->front;
    
    switch (thpool_p->jobqueue_p->len) {
        case 0:  /* if no jobs in queue */
            break;
        case 1:  /* if one job in queue */
            thpool_p->jobqueue_p->front = NULL;
            thpool_p->jobqueue_p->rear = NULL;
            thpool_p->jobqueue_p->len = 0;
            break;
        default: /* if more than one job in queue */
            thpool_p->jobqueue_p->front = job_p->prev;
            thpool_p->jobqueue_p->len--;
            /* more than one job in queue, post it */
            bsem_post(thpool_p->jobqueue_p->has_jobs);
            break;
    }

    return job_p;
}

/**
 * @brief clear the queue
 *
 * @param thpool_p
 */
void jobqueue_clear(thpool *thpool_p)
{
    while (thpool_p->jobqueue_p->len) {
        free(jobqueue_pull(thpool_p));
    }

    thpool_p->jobqueue_p->front = NULL;
    thpool_p->jobqueue_p->rear = NULL;
    bsem_reset(thpool_p->jobqueue_p->has_jobs);
    thpool_p->jobqueue_p->len = 0;
}

/**
 * @brief free all queue resources back to the system
 *
 * @param thpool_p [in] pointer to thread pool
 */
void jobqueue_destroy(thpool *thpool_p)
{
    jobqueue_clear(thpool_p);
    free(thpool_p->jobqueue_p->has_jobs);
}



/* ======================== THREAD ======================= */
/**
 * @brief sets the calling thread on hold
 */
static void thread_hold()
{
    threads_on_hold = 1;
    while (threads_on_hold) {
        sleep(1);
    }
}

/**
 * @brief what each thread is doing
 *
 * In principle this is an endless loop. The only time loop get interupped
 * is once thpool_destrpy() is invaked or the program exits.
 *
 * @param arg [in] thread that will run this function
 *
 * @return nothing 
 */
static void * thread_do(void *arg)
{
    if (arg == NULL) fprintf(stderr, "arg null\n");
    struct thread *thread_p = (struct thread *)arg;
    struct thpool *thpool_p = thread_p->thpool_p;

    /** 
     * register signal handler
     */
    struct sigaction act;
    act.sa_handler = thread_hold;
    if (sigaction(SIGUSR1, &act, NULL) == -1) {
        fprintf(stderr, "thread_do(): cannot handle SIGUSR1");
    }

    /**
     * Mark thread as alive (initialized)
     */
    pthread_mutex_lock(&thpool_p->thcount_lock);
    thpool_p->num_threads_alive += 1;
    pthread_mutex_unlock(&thpool_p->thcount_lock);

    while (threads_keepalive) {
        bsem_wait(thpool_p->jobqueue_p->has_jobs);

        if (threads_keepalive) {
            pthread_mutex_lock(&thpool_p->thcount_lock);
            thpool_p->num_threads_working++;
            pthread_mutex_unlock(&thpool_p->thcount_lock);

            /**
             * read job from queue and execute it
             */
            void* (*func_buff)(void *arg);
            void *arg_buff;
            job *job_p;
            pthread_mutex_lock(&thpool_p->thcount_lock);
            job_p = jobqueue_pull(thpool_p);
            pthread_mutex_unlock(&thpool_p->thcount_lock);

            if (job_p) {
                func_buff = job_p->function;
                arg_buff = job_p->arg;
                func_buff(arg_buff);
                free(job_p);
            }

            pthread_mutex_lock(&thpool_p->thcount_lock);
            thpool_p->num_threads_working--;
            pthread_mutex_unlock(&thpool_p->thcount_lock);
        }
    }

    pthread_mutex_lock(&thpool_p->thcount_lock);
    thpool_p->num_threads_alive -= 1;
    pthread_mutex_unlock(&thpool_p->thcount_lock);
    return NULL;
}

/**
 * @brief initialize a thread in the thread pool
 *
 * @param thpool_p [in]  pointer tp thread pool
 * @param thread_p [out] address to thre pointer of the thread to be created, if succ;
 * @param id       [in]  id to be given to the thread
 *
 */
void  thread_init(thpool *thpool_p, struct thread **thread_p)
{
    *thread_p = (struct thread *)malloc(sizeof(struct thread));
    if (thread_p == NULL) {
        fprintf(stderr, "Could not allocate memeory for thread\n");
        exit(1);
    }

    (*thread_p)->thpool_p = thpool_p;
    (*thread_p)->id = threads_id++;

    pthread_create(&(*thread_p)->pid, NULL, (void *)thread_do, *thread_p);
    pthread_detach((*thread_p)->pid);
}

/**
 * @brief frees a thread
 *
 * @param thread_p [in] pointer to thread
 */
void thread_destroy(thread *thread_p)
{
    free(thread_p);
}



/* ===================== THREAD POOL ===================== */
/**
 * @brief initialise thread pool
 *
 * @param num_threads
 *
 * @return 
 */
struct thpool * thpool_init(int num_threads)
{
    threads_on_hold = 0;
    threads_keepalive = 1;

    if (num_threads < 0) num_threads = 0;

    /**
     * make new thread pool
     */
    thpool *thpool_p;
    thpool_p = (struct thpool *)malloc(sizeof(struct thpool));
    if (thpool_p == NULL) {
        fprintf(stderr, "thpool_init(): Could not allocate memory for thread pool\n");
        return NULL;
    }
    thpool_p->num_threads_alive = 0;
    thpool_p->num_threads_working = 0;

    /**
     * initialise the job queue
     */
    if (jobqueue_init(thpool_p) == -1) {
        fprintf(stderr, "thpool_init(): Could not allocate memory for job queue\n");
        free(thpool_p);
        return NULL;
    }

    /**
     * make threads in pool
     */
    thpool_p->threads = (struct thread **)malloc(num_threads * sizeof(struct thread));
    if (thpool_p->threads == NULL) {
        fprintf(stderr, "thpool_init(): Could not allocate memory for threads\n");
        jobqueue_destroy(thpool_p);
        free(thpool_p->jobqueue_p);
        free(thpool_p);
        return NULL;
    }
    pthread_mutex_init(&thpool_p->thcount_lock, NULL);

    /**
     * thread init
     */
    int n;
    for (n = 0; n < num_threads; n++) {
        thread_init(thpool_p, &thpool_p->threads[n]);
        if (THPOOL_DEBUG)
            printf("THPOOL_DEBUG: Created thread %d in pool\n", n);
    }

    /**
     * wait for threads to initizlize
     */
    while (thpool_p->num_threads_alive != num_threads) usleep(10);

    return thpool_p;
}

/**
 * @brief add work to the thread pool
 *
 * @param thpool_p
 * @param function
 * @param arg
 *
 * @return 
 */
int thpool_add_work(thpool *thpool_p, void *(*function)(void*), void *arg)
{
    job *newjob;

    newjob = (struct job*)malloc(sizeof(struct job));
    if (newjob == NULL) {
        fprintf(stderr, "thpool_add_work(): Could not alloacte memory for new job\n");
        return -1;
    }

    /**
     * add function and argument
     */
    newjob->function = function;
    newjob->arg = arg;

    /**
     * add job to queue
     */
    pthread_mutex_lock(&thpool_p->jobqueue_p->rwlock);
    jobqueue_push(thpool_p, newjob);
    pthread_mutex_unlock(&thpool_p->jobqueue_p->rwlock);

    return 0;
}

/**
 * @brief wait until all jobs have finished
 *
 * @param thpool_p
 */
void thpool_wait(thpool *thpool_p)
{
    /**
     * continuous polling
     */
    double timeout = 1.0;
    double tpassed = 0.0;
    time_t start, end;

    time(&start);
    while (tpassed < timeout && 
            (thpool_p->jobqueue_p->len || thpool_p->num_threads_working))
    {
        time(&end);
        tpassed = difftime(end, start);
    }

    /**
     * exponential polling
     */
    long init_nano = 1;
    long new_nano = 0;
    double multiplier = 1.01;
    int max_secs = 20;

    struct timespec polling_interval;
    polling_interval.tv_sec = 0;
    polling_interval.tv_nsec = init_nano;

    while (thpool_p->jobqueue_p->len || thpool_p->num_threads_working)
    {
        nanosleep(&polling_interval, NULL);
        if (polling_interval.tv_sec < max_secs) {
            polling_interval.tv_sec = new_nano % MAX_NANOSEC;
            new_nano = CEIL(polling_interval.tv_sec * multiplier);
            if (new_nano > MAX_NANOSEC) {
                polling_interval.tv_sec++;
            }
        } else break;
    }

    /**
     * fall back to max polling
     */
    while (thpool_p->jobqueue_p->len || thpool_p->num_threads_working) {
        sleep(max_secs);
    }
}

/**
 * @brief destroy the thread pool
 *
 * @param thpool_p
 */
void thpool_destroy(thpool *thpool_p)
{
    volatile int threads_total = thpool_p->num_threads_alive;

    /**
     * end each thread's infinite loop
     */
    threads_keepalive = 0;

    /**
     * give one second to kill idle threads
     */
    double timeout = 1.0;
    double tpassed = 0.0;
    time_t start, end;
    time(&start);
    while (tpassed < timeout && 
            (thpool_p->num_threads_alive))
    {
        bsem_post_all(thpool_p->jobqueue_p->has_jobs);
        time(&end);
        tpassed = difftime(end, start);
    }

    /**
     * poll remaining threads
     */

    while (thpool_p->num_threads_alive)
    {
        bsem_post_all(thpool_p->jobqueue_p->has_jobs);
        sleep(1);
    }

    /**
     * job queue cleanup
     */
    jobqueue_destroy(thpool_p);
    free(thpool_p->jobqueue_p);

    /**
     * deallocs
     */
    int n;
    for (n = 0; n < threads_total; n++) {
        thread_destroy(thpool_p->threads[n]);
    }
    free(thpool_p->threads);
    free(thpool_p);
}

/**
 * @brief pause all threads in thread pool
 *
 * @param thpool_p
 */
void thpool_pause(thpool *thpool_p)
{
    int n;
    for (n = 0; n < thpool_p->num_threads_alive; n++) {
        pthread_kill(thpool_p->threads[n]->pid, SIGUSR1);
    }
}

/**
 * @brief resume all threads in thread poo;
 *
 * @param thpool_p
 */
void thpool_resume(thpool *thpool_p)
{
    threads_on_hold = 0;
}
