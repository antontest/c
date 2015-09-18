#include "thread.h"

static int thread_id = 1;
//static struct thread_queue *qthread = NULL;

/**
 * @brief pthread handler
 *
 * @param arg [in] struct pthread_impl pointer
 *
 * @return 
 */
void *thread_runtine(void *arg)
{
    thread_t *impl = (thread_t *)arg;
    thread_worker_t *worker = &impl->worker;

    impl->st = THREAD_RUNNING;
    while (impl->active) 
    {
        if (!impl->run) continue;

        if (worker->handler != NULL && impl->run)
        {
            worker->handler(worker->arg);

            worker->handler = NULL;
            worker->arg = NULL;
            impl->done = 1;
        }

        if (!impl->repeat) break;
        if (impl->delete) break;
    }

    if (impl->free.handler != NULL) impl->free.handler(arg);
    impl->delete = 1;
    impl->done = 1;

    pthread_mutex_destroy(&impl->lock);
    pthread_cond_destroy(&impl->ready);

    impl->st = THREAD_OVER;
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
int thread_start(thread_t *impl, thread_handler handler, void *arg, int run, int repeat)
{
    if (impl == NULL || impl->active) return -1;

    impl->st = THREAD_CREATING;
    impl->worker.handler = handler;
    impl->worker.arg = arg;

    impl->run = run;
    impl->repeat = repeat;
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
void thread_run(thread_t *impl)
{
    if (impl == NULL || !impl->active) return;

    impl->run = 1;
    impl->done = 0;
    impl->st = THREAD_RUNNING;

    return;
}

/**
 * @brief thread_stop 
 *
 * @param impl [in]
 */
void thread_stop(thread_t *impl)
{
    if (impl == NULL || !impl->active || !impl->run) return;
    
    impl->run = 0;
    impl->done = 0;
    impl->st = THREAD_STOPPED;

    return;
}

/**
 * @brief pthread exec another function
 *
 * @param impl [in]
 * @param pr   [in] callback
 * @param arg  [in]
 */
void thread_exec(thread_t *impl, thread_handler handler, void *arg)
{
    if (impl == NULL || !impl->active || !impl->repeat) return;

    while (!impl->done) usleep(10);
    impl->run = 0;
    impl->worker.handler = handler;
    impl->worker.arg = arg;

    impl->run = 1;
    impl->done = 0;

    return;
}

/**
 * @brief exec function when pthread over
 *
 * @param impl [in]
 * @param pr   [in] callback
 * @param arg  [in]
 */
void thread_on_exit(thread_t *impl, thread_handler handler, void *arg)
{
    if (impl == NULL || !impl->active || !impl->repeat) return;

    impl->free.handler = handler;
    impl->free.arg     = arg;

    return;
}


/**
 * @brief lock a thread
 * 
 * @param mtx [in] mutex
 *
 * @return 0, if succ; -1, if falied.
 */
int thread_lock(thread_t *impl)
{
    if (impl == NULL || impl->active) return -1;
    impl->st = THREAD_LOCK;

    return pthread_mutex_lock(&impl->lock);
}

/**
 * @brief lock a thread
 * 
 * @param mtx [in] mutex
 *
 * @return 0, if succ; -1, if falied.
 */
int thread_trylock(thread_t *impl)
{
    if (impl == NULL && !impl->active) return -1;
    impl->st = THREAD_LOCK;

    return pthread_mutex_trylock(&impl->lock);
}

/**
 * @brief unlock a thread
 * 
 * @param mtx [in] mutex
 *
 * @return 0, if succ; -1, if falied.
 */
int thread_unlock(thread_t *impl)
{
    if (impl == NULL && !impl->active) return -1;
    impl->st = THREAD_RUNNING;

    return pthread_mutex_unlock(&impl->lock);
}

/**
 * @brief destroy a lock of a thread
 * 
 * @param mtx [in] mutex
 *
 * @return 0, if succ; -1, if falied.
 */
int thread_delete(thread_t *impl)
{
    int wait_tm = 10 * 100;
    if (impl == NULL || !impl->active) return -1;

    /* wait over for current process */
    if (impl->run)
    {
        while(wait_tm-- > 0)
        {
            if (impl->done) break;
            else usleep(10);
        }
    }

    /* delete thread */
    impl->delete = 1;

    /* wait over for thread */
    if (impl->run)
    {
        while(wait_tm-- > 0)
        {
            if (impl->done) break;
            else usleep(10);
        }
    }

    /* if thread don't end, then call pthread_cancel to end it. */
    if (!impl->done)
    {
        pthread_cancel(impl->pid);
        pthread_mutex_destroy(&impl->lock);
        pthread_cond_destroy(&impl->ready);
    }
    impl->st = THREAD_OVER;

    return 0;
}

/**
 * @brief wait pthread over 
 *
 * @param impl [in]
 */
void thread_wait_over(thread_t *impl)
{
    if (impl == NULL || !impl->active) return;

    pthread_join(impl->pid, NULL);
    impl->st = THREAD_OVER;

    return;
}

/**
 * @brief wait pthread over 
 *
 * @param impl [in]
 */
void thread_time_wait_over(thread_t *impl, int tm_ms)
{
    struct timeval tv = {0};
    long timer_accuracy = 100;
    if (impl == NULL || !impl->active) return;

    while (1)
    {
        tv.tv_sec = 0;
        tv.tv_usec = 1000 * timer_accuracy;

        select(0, NULL, NULL, NULL, &tv);
        if (!impl->run) 
        {
            break;
        }

        tm_ms -= timer_accuracy;
        if (tm_ms <= 0) 
        {
            if (impl->run) pthread_cancel(impl->pid);
            break;
        }
    }

    impl->st = THREAD_OVER; 
    return;
}

/**
 * @brief thread_queue_handler 
 *
 * @param arg
 *
 * @return 
 */
void *thread_queue_handler(void *arg)
{
    thread_t *pthread = qthread->head;
    thread_t *pre = NULL, *pnext = NULL;

    while (1)
    {
        if (qthread->head == NULL) continue;

        /**
         * create thread
         */
        for (pthread = qthread->head; pthread != NULL; pthread = pthread->next)
        {
            if (!pthread->active && !pthread->done) {
                printf("create: %s\n", pthread->name);
                pthread->active = 1;
                pthread_create(&pthread->pid, NULL, thread_runtine, pthread);
            }
        }
        
        /**
         * free thread when thread free
         */
        pthread = qthread->head;
        pre = NULL;
        while (pthread != NULL)
        {
            pnext = pthread->next;

            if (!pthread->delete || pthread->hold) {
                pre = pthread;
                goto next;
            }

            pthread_mutex_lock(&qthread->lock);
            if (pthread == qthread->head) {
                qthread->head = pnext;
            } else if (pthread == qthread->tail) {
                qthread->tail = pre;
                qthread->tail->next = NULL;
            } else {
                pre->next = pnext;
            }

            printf("free: %s\n", pthread->name);
            free(pthread);
            pthread_mutex_unlock(&qthread->lock);
next:
            pthread = pnext;
        }
    }

    return NULL;

}

/**
 * @brief sig_deal 
 *
 * @param signum [in] number of signal
 */
static void sig_deal(int signum)
{
    thread_t *pthread = NULL;
    thread_t *p = NULL;
    
    switch (signum)
    {
        case SIGQUIT:
        case SIGHUP:
        case SIGKILL:
        case SIGTERM:
        case SIGINT:
        case SIGABRT:
            pthread_mutex_lock(&qthread->lock);
            pthread = qthread->head;
            while (pthread != NULL)
            {
                qthread->thread_total_cnt--;
                thread_id--;
                p = pthread;
                pthread = pthread->next;
                printf("sig free name: %s\n", p->name);
                free(p);
            }
            pthread_mutex_unlock(&qthread->lock);

            pthread_mutex_destroy(&qthread->lock);
            pthread_cond_destroy(&qthread->ready);
            if (qthread != NULL) free(qthread);
            qthread = NULL;
            exit(0);
            break;
        default:
            break;
    }
}

/**
 * @brief clean up when program over 
 */
static void exit_cleanup()
{
    thread_t *pthread = NULL;
    thread_t *p = NULL;
    
    if (qthread == NULL) return;
    pthread_mutex_lock(&qthread->lock);
    pthread = qthread->head;
    while (pthread != NULL)
    {
        qthread->thread_total_cnt--;
        p = pthread;
        printf("exit free name: %s\n", p->name);
        pthread = pthread->next;
        thread_id--;
        free(p);
    }
    pthread_mutex_unlock(&qthread->lock);

    pthread_mutex_destroy(&qthread->lock);
    pthread_cond_destroy(&qthread->ready);
    free(qthread);
    qthread = NULL;
}

/**
 * @brief thread_create
 *
 * @param name    [in] name of thread
 * @param handler [in] callback
 * @param arg     [in] arg
 * @param run     [in] whether run at the time of creating
 * @param repeat    [in] whether run cycly
 *
 * @return pthread_id, if succ; -1, if failed 
 */
int thread_create(const char *name, thread_handler handler, void *arg, 
        int run, int repeat)
{
    struct thread_impl *new_thread = NULL;
    struct sigaction sa;

    if (name == NULL)   return -1;
    if (qthread == NULL) { 
        qthread = (struct thread_queue *)malloc(sizeof(struct thread_queue));
        memset(qthread, 0, sizeof(struct thread_queue));
    }
    if (qthread == NULL) return -1;
    if ((new_thread = (struct thread_impl *)malloc(sizeof(struct thread_impl))) == NULL)
        return -1;

    memset(new_thread, 0, sizeof(struct thread_impl));
    new_thread->name  = name;
    new_thread->id    = thread_id++;
    new_thread->worker.handler    = handler;
    new_thread->worker.arg        = arg;

    new_thread->run       = run;
    new_thread->repeat    = repeat;
    new_thread->done      = 0;
    new_thread->delete    = 0;
    new_thread->hold      = 0;
    new_thread->st        = THREAD_CREATING;

    pthread_mutex_init(&new_thread->lock, NULL);
    pthread_cond_init(&new_thread->ready, NULL);

    if (qthread->pid <= 0) {
        pthread_mutex_init(&qthread->lock, NULL);
        pthread_cond_init(&qthread->ready, NULL);
    }

    /**
     * add thread
     */
    pthread_mutex_lock(&qthread->lock);
    if (qthread->head == NULL)
        qthread->head = new_thread;
    else qthread->tail->next = new_thread;
    qthread->tail        = new_thread;
    qthread->tail->next  = NULL;
    qthread->thread_total_cnt++;
    pthread_mutex_unlock(&qthread->lock);

    /**
     * create thread
     */
    if (qthread->pid <= 0) {
        /**
         * act signal to queue
         */
        sa.sa_flags = 0;
        sa.sa_handler = sig_deal;
        sigaction(SIGKILL, &sa, NULL);
        sigaction(SIGTERM, &sa, NULL);
        sigaction(SIGQUIT, &sa, NULL);
        sigaction(SIGINT, &sa, NULL);
        sigaction(SIGHUP, &sa, NULL);
        sigaction(SIGABRT, &sa, NULL);

        atexit(exit_cleanup);
        pthread_create(&qthread->pid, NULL, thread_queue_handler, qthread);
    }

    return new_thread->id;
}

/**
 * @brief pthread_start 
 *
 * @param cfg [in] thread configure
 *
 * @return pthread_id, if succ; -1, if failed
 */
int pthread_start(struct thread_cfg *cfg)
{
    struct thread_impl *new_thread = NULL;
    struct sigaction sa;

    if (cfg == NULL)   return -1;
    if (qthread == NULL) { 
        qthread = (struct thread_queue *)malloc(sizeof(struct thread_queue));
        memset(qthread, 0, sizeof(struct thread_queue));
    }
    if (qthread == NULL) return -1;
    if ((new_thread = (struct thread_impl *)malloc(sizeof(struct thread_impl))) == NULL)
        return -1;

    memset(new_thread, 0, sizeof(struct thread_impl));
    new_thread->name   = cfg->name;
    new_thread->id     = thread_id++;
    new_thread->worker = cfg->worker;
    new_thread->free   = cfg->free;

    new_thread->run    = cfg->run;
    new_thread->repeat = cfg->repeat;
    new_thread->done   = 0;
    new_thread->delete = 0;
    new_thread->hold   = 0;

    pthread_mutex_init(&new_thread->lock, NULL);
    pthread_cond_init(&new_thread->ready, NULL);

    if (qthread->pid <= 0) {
        pthread_mutex_init(&qthread->lock, NULL);
        pthread_cond_init(&qthread->ready, NULL);
    }

    /**
     * add thread
     */
    pthread_mutex_lock(&qthread->lock);
    if (qthread->head == NULL)
        qthread->head = new_thread;
    else qthread->tail->next = new_thread;
    qthread->tail        = new_thread;
    qthread->tail->next  = NULL;
    qthread->thread_total_cnt++;
    pthread_mutex_unlock(&qthread->lock);

    /**
     * create thread
     */
    if (qthread->pid <= 0) {
        /**
         * act signal to queue
         */
        sa.sa_flags = 0;
        sa.sa_handler = sig_deal;
        sigaction(SIGKILL, &sa, NULL);
        sigaction(SIGTERM, &sa, NULL);
        sigaction(SIGQUIT, &sa, NULL);
        sigaction(SIGINT , &sa, NULL);
        sigaction(SIGHUP , &sa, NULL);
        sigaction(SIGABRT, &sa, NULL);

        atexit(exit_cleanup);
        pthread_create(&qthread->pid, NULL, thread_queue_handler, qthread);
    }

    return new_thread->id;
}

/**
 * @brief get_thread 
 *
 * @param thread_idi [in] thread id
 *
 * @return thread impl, if succ; NULL, if failed
 */
struct thread_impl * get_pthread(int thread_id)
{
    struct thread_impl *pthread = qthread->head;

    while (pthread != NULL && pthread->id != thread_id)
        pthread = pthread->next;

    return pthread;
}

/**
 * @brief pthread_run 
 *
 * @param thread_id [in] thread id
 */
void pthread_run(int thread_id)
{
    thread_t *pthread = get_pthread(thread_id);

    if (pthread != NULL) {
        pthread->run = 1;
        pthread->st  = THREAD_RUNNING;
    }
}

/**
 * @brief pthread_stop 
 *
 * @param thread_id [in] thread id
 */
void pthread_stop(int thread_id)
{
    thread_t *pthread = get_pthread(thread_id);
    if (pthread != NULL) {
        pthread->run = 0;
        pthread->st  = THREAD_STOPPED;
    }
}

/**
 * @brief pthread_delete
 *
 * @param thread_id [in] thread id
 */
void pthread_delete(int thread_id)
{
    thread_t *pthread = get_pthread(thread_id);

    if (pthread != NULL) {
        pthread->delete = 1;
        pthread->st = THREAD_OVER;
    }
}

/**
 * @brief pthread_hold 
 *
 * @param thread_id [in] thread id
 */
void pthread_hold(int thread_id)
{
    thread_t *pthread = get_pthread(thread_id);

    if (pthread != NULL) {
        pthread->hold = 1;
    }
}

/**
 * @brief pthread_unhold 
 *
 * @param thread_id [in] thread id
 */
void pthread_unhold(int thread_id)
{
    thread_t *pthread = get_pthread(thread_id);

    if (pthread != NULL) {
        pthread->hold = 0;
    }
}

/**
 * @brief pthread_lock 
 *
 * @param thread_id [in] thread id
 */
void pthread_lock(int thread_id)
{
    thread_t *pthread = get_pthread(thread_id);

    if (pthread != NULL) {
        pthread->st = THREAD_LOCK;
        pthread_mutex_lock(&pthread->lock);
    }
}

/**
 * @brief pthread_unlock 
 *
 * @param thread_id [in] thread id
 */
void pthread_unlock(int thread_id)
{
    thread_t *pthread = get_pthread(thread_id);

    if (pthread != NULL) {
        pthread->st = THREAD_RUNNING;
        pthread_mutex_unlock(&pthread->lock);
    }
}

/**
 * @brief pthread_wait 
 *
 * @param thread_id [in] thread id
 */
void pthread_wait(int thread_id)
{
    thread_t *pthread = get_pthread(thread_id);

    if (pthread != NULL && pthread->st == THREAD_LOCK) {
        pthread->st = THREAD_WAIT;
        pthread_cond_wait(&pthread->ready, &pthread->lock);
    }
}

/**
 * @brief pthread_unwait 
 *
 * @param thread_id [in] thread id
 */
void pthread_unwait(int thread_id)
{
    thread_t *pthread = get_pthread(thread_id);

    if (pthread != NULL) {
        pthread->st = THREAD_RUNNING;
        pthread_cond_signal(&pthread->ready);
    }
}

