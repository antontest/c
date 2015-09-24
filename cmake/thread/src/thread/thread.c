#include "thread.h"

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

/**
 * @brief cleanup_runtine -- happened thread be canceled 
 *
 * @param arg
 */
void cleanup_runtine(void *arg)
{
    struct thread *thread_p = NULL;
    if (arg == NULL) return;

    /**
     * default thread cancel dealing
     */
    thread_p = (struct thread *)arg;
    if (thread_p->clean.function != NULL)
        thread_p->clean.function(thread_p->clean.arg);
    thread_p->run = 0;
    thread_p->delete = 1;
    thread_p->state = THREAD_OVER;
    //printf("thread %d be canceled, start to clean\n", thread_p->id);

    return;
}

/**
 * @brief thread_p handler
 *
 * @param arg [in] struct thread_p pointer
 *
 * @return 
 */
void *thread_runtine(void *arg)
{
    /**
     * thread parameter init
     */
    struct thread *thread_p = NULL;
    if (arg == NULL) return NULL;
    thread_p = (struct thread *)arg;
    struct job *worker = &thread_p->worker;
    thread_p->state = THREAD_RUNNING;

    /**
     * set thread can be canceled
     */
    enable_cancel();
    set_cancel_defe();
    pthread_cleanup_push(cleanup_runtine, arg);
    pthread_testcancel();

    /**
     * thread thread dealing function
     */
    while (thread_p->active) 
    {
        pthread_mutex_lock(&thread_p->lock);
        if (thread_p->delete) break;
        if (!thread_p->run) {
            pthread_mutex_unlock(&thread_p->lock);
            continue;
        }

        /**
         * thread_p task handle
         */
        if (worker->function != NULL) {
            /**
             * start execute task, update thread state
             */
            thread_p->state = THREAD_BUSY;
            worker->function(worker->arg);

            /**
             * clear callback, if no repeat
             * update thread state
             */
            if (!thread_p->repeat) {
                worker->arg = NULL;
                worker->function = NULL;
                
                /**
                 * update thread state
                 */
                thread_p->run = 0;
                thread_p->state = THREAD_IDLE;
            }
        }
        pthread_mutex_unlock(&thread_p->lock);

        if (!thread_p->hold) break;
    }
    pthread_mutex_unlock(&thread_p->lock);

    //printf("thread %d over\n", thread_p->id);
    if (thread_p->clean.function != NULL) 
        thread_p->clean.function(thread_p->clean.arg);
    thread_p->run = 0;
    thread_p->delete = 1;
    thread_p->state = THREAD_OVER;
    pthread_cleanup_pop(0);

    return NULL;
}


/******************************************************
 *************  thread_p Queue Function  ***************
 ******************************************************/

/**
 * @brief get_localtime 
 *
 * @return 
 */
/*
static long get_localtime()
{
    struct sysinfo sys_tm = {0};

    sysinfo(&sys_tm);
    return sys_tm.uptime;
}
*/



/******************************************************
*************** thread_p Clear Function ****************
******************************************************/



/******************************************************
*************** thread_p Basic Function ****************
******************************************************/
/**
 * @brief pcreate 
 *
 * @param handler [in] callback
 * @param arg     [in] arg
 *
 * @return pid, if succ; -1, if failed
 */
pthread_t pcreate(thread_handler handler, void *arg)
{
    pthread_t pid = 0;

    pthread_create(&pid, NULL, handler, arg);
    return pid;
}

/**
 * @brief lock -- lock thread
 *
 * @param mtx [in] thread_p mutex
 */
void plock(pthread_mutex_t *mtx)
{
    if (mtx == NULL) return;
    pthread_mutex_lock(mtx);
}

/**
 * @brief ptrylock -- lock thread
 *
 * @param mtx [in] thread_p mutex
 */
void ptrylock(pthread_mutex_t *mtx)
{
    if (mtx == NULL) return;
    pthread_mutex_trylock(mtx);
}

/**
 * @brief unlock -- unlock thread
 *
 * @param mtx [in] thread_p mutex
 */
void punlock(pthread_mutex_t *mtx)
{
    if (mtx == NULL) return;
    pthread_mutex_unlock(mtx);
}

/**
 * @brief wait -- wait another thread
 *
 * @param cond [in] thread_p cond
 * @param mtx  [in] pthred mutex
 */
void pwait(pthread_cond_t *cond, pthread_mutex_t *mtx)
{
    if (cond == NULL || mtx) return;
    pthread_cond_wait(cond, mtx);
}

/**
 * @brief pcontinue -- let another thread go on
 *
 * @param cond [in] thread_p cond
 */
void pcontinue(pthread_cond_t *cond)
{
    if (cond == NULL) return;
    pthread_cond_signal(cond);
}

/**
 * @brief wait thread_p over 
 *
 * @param thread_p [in]
 */
void pjoin(pthread_t pid)
{
    pthread_join(pid, NULL);

    return;
}

/**
 * @brief pexit -- wait for thread over safely 
 *
 * @param rval [out] thread return value
 */
void pexit(void *rval)
{
    pthread_exit(rval);
}

/**
 * @brief pkill -- kill thread
 *
 * @param pid    [in] pid
 * @param signal [in] signal number
 *
 * @return 0, if thread exit and succ; EINVAL or ESRCH, if failed
 */
int pkill(pthread_t pid, int signal)
{
    return pthread_kill(pid, signal);
}

/**
 * @brief pcancel 
 *
 * @param pid [in] id
 *
 * @return 
 */
int pcancel(pthread_t pid)
{
    return pthread_cancel(pid);
}


/******************************************************
*************** thread_p Attribute Function ************
******************************************************/
/**
 * @brief enable_cancel 
 */
int enable_cancel()
{
    return pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
}

/**
 * @brief enable_cancel 
 */
int disable_cancel()
{
    return pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
}

/**
 * @brief set_cancel_asyn 
 */
int set_cancel_asyn()
{
    return pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
}

/**
 * @brief set_cancel_asyn 
 */
int set_cancel_defe()
{
    return pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
}

/**
 * @brief set_joinable -- set thread joinable
 *
 * @return 0, if succ; -1, if failed
 */
int set_joinable()
{
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    return pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
}

/**
 * @brief set_detach -- set thread detached
 *
 * @return 0, if succ; -1, if failed
 */
int set_detach()
{
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    return pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
}

/******************************************************
 *************** thread_p Manage Function **************
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
void bsem_init(bsem *pbsem, int value)
{
    if (value < 0 || value > 1) {
        fprintf(stderr, "bsem_init(): Binary semaphore can take only values 1 or 0");
        exit(1);
    }

    pthread_mutex_init(&pbsem->mutex, NULL);
    pthread_cond_init(&pbsem->cond, NULL);
    pbsem->v = value;
}

/**
 * @brief reset semaphore to 0
 *
 * @param pbsem [in] semaphore
 */
void bsem_reset(bsem *pbsem)
{
    bsem_init(pbsem, 0);
}

/**
 * @brief post to at least one thread
 *
 * @param pbsem
 */
void bsem_post(bsem *pbsem)
{
    pthread_mutex_lock(&pbsem->mutex);
    pbsem->v = 1;
    pthread_cond_signal(&pbsem->cond);
    pthread_mutex_unlock(&pbsem->mutex);
}

/**
 * @brief post all threads
 *
 * @param pbsem
 */
void bsem_post_all(bsem *pbsem)
{
    pthread_mutex_lock(&pbsem->mutex);
    pbsem->v = 1;
    pthread_cond_broadcast(&pbsem->cond);
    pthread_mutex_unlock(&pbsem->mutex);
}

/**
 * @brief wait on semaphore until semaphore has value 0
 *
 * @param pbsem
 */
void bsem_wait(bsem *pbsem)
{
    pthread_mutex_lock(&pbsem->mutex);
    while (pbsem->v != 1) {
        pthread_cond_wait(&pbsem->cond, &pbsem->mutex);
    }
    pthread_mutex_unlock(&pbsem->mutex);
}


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
    pthread_mutex_lock(&(thpool_p->thcount_lock));
    thpool_p->num_threads_alive += 1;
    pthread_mutex_unlock(&(thpool_p->thcount_lock));

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

    pthread_create(&(*thread_p)->pid, NULL, (void *)thread_do, thread_p);
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
