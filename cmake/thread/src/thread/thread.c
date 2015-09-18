#include "thread.h"

static int thread_id = 1;
static int cleanup_event_init = 0;
static int on_exit_event = 1;

/**
 * @brief pthread handler
 *
 * @param arg [in] struct pthread pointer
 *
 * @return 
 */
void *thread_runtine(void *arg)
{
    thread_t *impl = (thread_t *)arg;
    thread_worker_t *worker = &impl->worker;

    while (impl->active) 
    {
        if (impl->delete) break;
        if (!impl->run) continue;

        if (worker->handler != NULL) {
            impl->st = THREAD_RUNNING;
            worker->handler(worker->arg);
            if (!impl->repeat) {
                worker->handler = NULL;
                worker->arg = NULL;
                impl->st = THREAD_IDLE;
                impl->run = 0;
                if (pool != NULL) {
                    pool->thread_idle_cnt++;
                }
            }
        }

        if (!impl->hold) break;
    }

    printf("thread over\n");
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
int thread_start(thread_t *impl, thread_handler handler, void *arg, 
        int run, int repeat)
{
    if (impl == NULL || impl->active) return -1;

    impl->st = THREAD_CREATING;
    impl->worker.handler = handler;
    impl->worker.arg = arg;

    impl->run    = run;
    impl->repeat = repeat;
    impl->done   = 0;
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
 * @brief get_localtime 
 *
 * @return 
 */
static long get_localtime()
{
    struct sysinfo sys_tm = {0};

    sysinfo(&sys_tm);
    return sys_tm.uptime;
}

/**
 * @brief thread_pool_handler 
 *
 * @param arg
 *
 * @return 
 */
void *thread_pool_handler(void *arg)
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
            if (qthread != NULL) {
                pthread_mutex_lock(&qthread->lock);
                pthread = qthread->head;
                while (pthread != NULL)
                {
                    qthread->thread_total_cnt--;
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
            }

            if (pool != NULL) {
                pthread_mutex_lock(&pool->lock);
                pthread = pool->head;
                while (pthread != NULL)
                {
                    pool->thread_total_cnt--;
                    p = pthread;
                    pthread = pthread->next;
                    printf("sig free name: %s\n", p->name);
                    free(p);
                }
                pthread_mutex_unlock(&pool->lock);

                pthread_mutex_destroy(&pool->lock);
                pthread_cond_destroy(&pool->ready);
                if (pool != NULL) free(pool);
                pool = NULL;
            }

            exit(0);
            break;
        default:
            break;
    }
}

static void thread_cleanup_event_add()
{
    struct sigaction sa;

    if (cleanup_event_init) return;

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
    cleanup_event_init = 1;

    return;
}

/**
 * @brief clean up when program over 
 */
static void exit_cleanup()
{
    thread_t *pthread = NULL;
    thread_t *p = NULL;

    on_exit_event = 1;
    if (qthread != NULL) {
        pthread_mutex_lock(&qthread->lock);
        pthread = qthread->head;
        while (pthread != NULL)
        {
            qthread->thread_total_cnt--;
            p = pthread;
            printf("qthread exit free name: %s\n", p->name);
            pthread = pthread->next;
            free(p);
        }
        pthread_mutex_unlock(&qthread->lock);

        pthread_mutex_destroy(&qthread->lock);
        pthread_cond_destroy(&qthread->ready);
        free(qthread);
        qthread = NULL;
    }

    if (pool != NULL) {
        pthread_mutex_lock(&pool->lock);
        pthread = pool->head;
        while (pthread != NULL)
        {
            pool->thread_total_cnt--;
            p = pthread;
            printf("pool exit free name: %s\n", p->name);
            pthread = pthread->next;
            free(p);
        }
        pthread_mutex_unlock(&pool->lock);

        pthread_mutex_destroy(&pool->lock);
        pthread_cond_destroy(&pool->ready);
        free(pool);
        pool = NULL;
    }

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
    struct thread *new_thread = NULL;

    if (name == NULL)   return -1;
    if (qthread == NULL) { 
        qthread = (struct thread_pool *)malloc(sizeof(struct thread_pool));
        memset(qthread, 0, sizeof(struct thread_pool));
    }
    if (qthread == NULL) return -1;
    if ((new_thread = (struct thread *)malloc(sizeof(struct thread))) == NULL)
        return -1;

    memset(new_thread, 0, sizeof(struct thread));
    new_thread->name  = name;
    new_thread->id    = thread_id++;
    new_thread->create_time       = get_localtime();
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
     * pthread clean up
     */
    thread_cleanup_event_add();
    atexit(exit_cleanup);

    /**
     * create thread
     */
    if (qthread->pid <= 0) {
        pthread_create(&qthread->pid, NULL, thread_pool_handler, qthread);
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
    struct thread *new_thread = NULL;

    if (cfg == NULL)   return -1;
    if (qthread == NULL) { 
        qthread = (struct thread_pool *)malloc(sizeof(struct thread_pool));
        memset(qthread, 0, sizeof(struct thread_pool));
    }
    if (qthread == NULL) return -1;
    if ((new_thread = (struct thread *)malloc(sizeof(struct thread))) == NULL)
        return -1;

    memset(new_thread, 0, sizeof(struct thread));
    new_thread->name   = cfg->name;
    new_thread->id     = thread_id++;
    new_thread->worker = cfg->worker;
    new_thread->free   = cfg->free;

    new_thread->run    = cfg->run;
    new_thread->repeat = cfg->repeat;
    new_thread->done   = 0;
    new_thread->delete = 0;
    new_thread->hold   = 0;
    new_thread->st     = THREAD_CREATING;
    new_thread->create_time = get_localtime();

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
     * pthread clean up
     */
    thread_cleanup_event_add();
    atexit(exit_cleanup);

    /**
     * create thread
     */
    if (qthread->pid <= 0) {
        atexit(exit_cleanup);
        pthread_create(&qthread->pid, NULL, thread_pool_handler, qthread);
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
struct thread * get_pthread(int thread_id)
{
    struct thread *pthread = qthread->head;

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

/**
 * @brief pthread_info 
 */
void pthread_info()
{
    thread_t *pthread = NULL;
    const char *state[] = {   
        "idle", 
        "creating", 
        "running", 
        "stopped", 
        "locked", 
        "waiting", 
        "over"
    };
 
    if (qthread != NULL) {
        for (pthread = qthread->head; pthread != NULL; 
                pthread = pthread->next)
        {
            printf("qthread id: %d ---> name: %s ---> status: %s\n", 
                    pthread->id, pthread->name, state[pthread->st]);
        }
    }

    if (pool != NULL) {
        for (pthread = pool->head; pthread != NULL; 
                pthread = pthread->next)
        {
            printf("pool id: %d ---> name: %s ---> status: %s\n", 
                    pthread->id, pthread->name, state[pthread->st]);
        }
    }
}

/**
 * @brief pthread_pool_runtine 
 *
 * @param arg
 *
 * @return 
 */
static void * pthread_pool_runtine(void *arg)
{
    struct thread *pthread = NULL;
    struct thread *pre = NULL, *pnext = NULL;
    long free_time = 5;

    while (pool->active) 
    {
        /**
         * free thread when thread free
         */
        pthread = pool->head;
        pre = NULL;
        while (pthread != NULL)
        {
            pnext = pthread->next;

            if (pool->thread_idle_cnt <= pool->thread_mini_cnt) goto next;
            if ((pthread->active && pthread->st != THREAD_IDLE) || 
                ((get_localtime() - pthread->create_time) < free_time) 
                ) 
            {
                pre = pthread;
                goto next;
            }

            pthread_mutex_lock(&pool->lock);
            if (pthread == pool->head) {
                pool->head = pnext;
            } else if (pthread == pool->tail) {
                pool->tail = pre;
                pool->tail->next = NULL;
            } else {
                pre->next = pnext;
            }

            printf("pool timeout free: %s\n", pthread->name);
            free(pthread);
            pool->thread_idle_cnt--; 
            pool->thread_total_cnt--; 
            pthread_mutex_unlock(&pool->lock);
next:
            pthread = pnext;
        }
    }

    return NULL;
}

/**
 * @brief pthread_pool_init 
 *
 * @param max_cnt  [in] mac count of thread in pthread pool
 * @param init_cnt [in] init count of thread in pthread pool
 *
 * @return 0, if succ; -1, if failed
 */
int pthread_pool_init(int max_cnt, int mini_cnt, int init_cnt)
{
    struct thread *pthread = NULL;

    /**
     * init pthred pool
     */
    if (pool != NULL) return 0;
    pool = (struct thread_pool *)malloc(sizeof(struct thread_pool));
    if (pool == NULL) return -1;
    memset(pool, 0, sizeof(struct thread_pool));
    pool->thread_max_cnt = max_cnt;
    pool->thread_mini_cnt = mini_cnt;
    pool->active = 1;
    
    /**
     * create thread of pool manager
     */
    pthread_mutex_init(&pool->lock, NULL);
    pthread_cond_init(&pool->ready, NULL);
    pthread_create(&pool->pid, NULL, pthread_pool_runtine, pool);

    /**
     * create thread for pthread pool
     */
    while (init_cnt-- > 0)
    {
        pthread = (struct thread *)malloc(sizeof(struct thread));
        if (pthread == NULL) continue;
        memset(pthread, 0, sizeof(struct thread));
        pthread->id = thread_id++;
        pthread->active = 1;
        pthread->hold = 1;
        pthread->st = THREAD_CREATING;
        pthread->create_time = get_localtime();

        if (pthread_create(&pthread->pid, NULL, thread_runtine, pthread) < 0)
        {
            free(pthread);
            pthread = NULL;
            continue;
        }

        if (pool->head == NULL) {
            pool->head = pthread;
        } else {
            pool->tail->next = pthread;
        }
        pool->tail = pthread;
        pool->thread_total_cnt++;
        pool->thread_idle_cnt++;
        usleep(10);
    }

    /**
     * pthread clean up
     */
    thread_cleanup_event_add();
    atexit(exit_cleanup);

    return 0;
}

/**
 * @brief get_idle_thread 
 *
 * @return thread, if succ; NULL, if failed
 */
struct thread * get_idle_thread()
{
    struct thread *pthread = NULL;

    for (pthread = pool->head; pthread != NULL; 
        pthread = pthread->next)
    {
        if (pthread->worker.handler == NULL) break;
    }

    return pthread;
}

/**
 * @brief pthread_pool_add 
 *
 * @param handler [in] pthread callback
 * @param arg     [in]
 *
 * @return 0, if succ; -1, if failed
 */
int pthread_pool_add(thread_handler handler, void *arg)
{
    struct thread *pthread = NULL;
    int i = 0;

    while (!pool->active) usleep(100);
    if (pool->thread_total_cnt > pool->thread_max_cnt) 
        return -1;

    /**
     * search idle thread
     */
    if (pool->thread_idle_cnt > 0)
    {
        pthread = get_idle_thread();
        if (pthread == NULL) return -1;

        pthread_mutex_lock(&pool->lock);
        pool->thread_idle_cnt--;
        pthread_mutex_unlock(&pool->lock);
        
        pthread->run = 1;
        pthread->worker.handler = handler;
        pthread->worker.arg = arg;
        usleep(10);
        return 0;
    }

    /**
     * create new thread
     */
    while (i++ < pool->thread_mini_cnt + 1 && 
           pool->thread_total_cnt <= pool->thread_max_cnt)
    {
        pthread = (struct thread *)malloc(sizeof(struct thread));
        if (pthread == NULL) return -1;
        memset(pthread, 0, sizeof(struct thread));
        pthread->id = thread_id++;
        pthread->active = 1;
        pthread->hold = 1;
        pthread->st = THREAD_CREATING;
        pthread->create_time = get_localtime();

        if (pthread_create(&pthread->pid, NULL, thread_runtine, pthread) < 0)
        {
            free(pthread);
            pthread = NULL;
            return -1;
        }
        pool->thread_idle_cnt++;
        pool->thread_total_cnt++;

        /**
         * add thread to pthread pool
         */
        pthread_mutex_lock(&pool->lock);
        if (pool->head == NULL) {
            pool->head = pthread;
        } else {
            pool->tail->next = pthread;
        }
        pool->tail = pthread;

        pthread_mutex_unlock(&pool->lock);
        usleep(10);
    }

    if (pool->thread_idle_cnt < 1) return -1;

    pthread->run = 1;
    pthread->worker.handler = handler;
    pthread->worker.arg = arg;
    pthread_mutex_lock(&pool->lock);
    pool->thread_idle_cnt--;
    pthread_mutex_unlock(&pool->lock);
    usleep(10);

    return 0;
}
