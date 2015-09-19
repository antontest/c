#include "thread.h"

static int thread_id = 1;
static int cleanup_event_init = 0;

/**
 * @brief pthread handler
 *
 * @param arg [in] struct pthread pointer
 *
 * @return 
 */
void *thread_runtine(void *arg)
{
    thread_t *pthread = (thread_t *)arg;
    thread_worker_t *worker = &pthread->worker;

    pthread->st = THREAD_RUNNING;
    while (pthread->active) 
    {
        if (pthread->delete) break;
        if (!pthread->run) continue;

        if (worker->handler != NULL) {
            pthread->st = THREAD_BUSY;
            worker->handler(worker->arg);
            if (!pthread->repeat) {
                worker->handler = NULL;
                worker->arg = NULL;
                pthread->st = THREAD_IDLE;
                pthread->run = 0;
            }
        }

        if (!pthread->hold) break;
    }

    printf("thread over\n");
    if (pthread->free.handler != NULL) pthread->free.handler(arg);
    pthread->delete = 1;
    pthread->done = 1;

    pthread_mutex_destroy(&pthread->lock);
    pthread_cond_destroy(&pthread->ready);

    pthread->st = THREAD_OVER;
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
int thread_start(thread_t *pthread, thread_handler handler, void *arg, 
        int run, int repeat)
{
    if (pthread == NULL || pthread->active) return -1;

    pthread->st = THREAD_CREATING;
    pthread->worker.handler = handler;
    pthread->worker.arg = arg;

    pthread->run    = run;
    pthread->repeat = repeat;
    pthread->done   = 0;
    pthread->delete = 0;
    pthread->active = 1;

    pthread_mutex_init(&pthread->lock, NULL);
    pthread_cond_init(&pthread->ready, NULL);

    return pthread_create(&pthread->pid, NULL, thread_runtine, pthread);
}

/**
 * @brief let pthread run
 *
 * @param pthread [in] 
 */
void thread_run(thread_t *pthread)
{
    if (pthread == NULL || !pthread->active) return;

    pthread->run = 1;
    pthread->done = 0;
    pthread->st = THREAD_RUNNING;

    return;
}

/**
 * @brief thread_stop 
 *
 * @param pthread [in]
 */
void thread_stop(thread_t *pthread)
{
    if (pthread == NULL || !pthread->active || !pthread->run) return;
    
    pthread->run = 0;
    pthread->done = 0;
    pthread->st = THREAD_STOPPED;

    return;
}

/**
 * @brief pthread exec another function
 *
 * @param pthread [in]
 * @param pr   [in] callback
 * @param arg  [in]
 */
void thread_exec(thread_t *pthread, thread_handler handler, void *arg)
{
    if (pthread == NULL || !pthread->active || !pthread->repeat) return;

    while (!pthread->done) usleep(10);
    pthread->run = 0;
    pthread->worker.handler = handler;
    pthread->worker.arg = arg;

    pthread->run = 1;
    pthread->done = 0;

    return;
}

/**
 * @brief exec function when pthread over
 *
 * @param pthread [in]
 * @param pr   [in] callback
 * @param arg  [in]
 */
void thread_on_exit(thread_t *pthread, thread_handler handler, void *arg)
{
    if (pthread == NULL || !pthread->active || !pthread->repeat) return;

    pthread->free.handler = handler;
    pthread->free.arg     = arg;

    return;
}


/**
 * @brief lock a thread
 * 
 * @param mtx [in] mutex
 *
 * @return 0, if succ; -1, if falied.
 */
int thread_lock(thread_t *pthread)
{
    if (pthread == NULL || pthread->active) return -1;
    pthread->st = THREAD_LOCK;

    return pthread_mutex_lock(&pthread->lock);
}

/**
 * @brief lock a thread
 * 
 * @param mtx [in] mutex
 *
 * @return 0, if succ; -1, if falied.
 */
int thread_trylock(thread_t *pthread)
{
    if (pthread == NULL && !pthread->active) return -1;
    pthread->st = THREAD_LOCK;

    return pthread_mutex_trylock(&pthread->lock);
}

/**
 * @brief unlock a thread
 * 
 * @param mtx [in] mutex
 *
 * @return 0, if succ; -1, if falied.
 */
int thread_unlock(thread_t *pthread)
{
    if (pthread == NULL && !pthread->active) return -1;
    pthread->st = THREAD_RUNNING;

    return pthread_mutex_unlock(&pthread->lock);
}

/**
 * @brief destroy a lock of a thread
 * 
 * @param mtx [in] mutex
 *
 * @return 0, if succ; -1, if falied.
 */
int thread_delete(thread_t *pthread)
{
    int wait_tm = 10 * 100;
    if (pthread == NULL || !pthread->active) return -1;

    /* wait over for current process */
    if (pthread->run)
    {
        while(wait_tm-- > 0)
        {
            if (pthread->done) break;
            else usleep(10);
        }
    }

    /* delete thread */
    pthread->delete = 1;

    /* wait over for thread */
    if (pthread->run)
    {
        while(wait_tm-- > 0)
        {
            if (pthread->done) break;
            else usleep(10);
        }
    }

    /* if thread don't end, then call pthread_cancel to end it. */
    if (!pthread->done)
    {
        pthread_cancel(pthread->pid);
        pthread_mutex_destroy(&pthread->lock);
        pthread_cond_destroy(&pthread->ready);
    }
    pthread->st = THREAD_OVER;

    return 0;
}

/**
 * @brief wait pthread over 
 *
 * @param pthread [in]
 */
void thread_wait_over(thread_t *pthread)
{
    if (pthread == NULL || !pthread->active) return;

    pthread_join(pthread->pid, NULL);
    pthread->st = THREAD_OVER;

    return;
}

/**
 * @brief wait pthread over 
 *
 * @param pthread [in]
 */
void thread_time_wait_over(thread_t *pthread, int tm_ms)
{
    struct timeval tv = {0};
    long timer_accuracy = 100;
    if (pthread == NULL || !pthread->active) return;

    while (1)
    {
        tv.tv_sec = 0;
        tv.tv_usec = 1000 * timer_accuracy;

        select(0, NULL, NULL, NULL, &tv);
        if (!pthread->run) 
        {
            break;
        }

        tm_ms -= timer_accuracy;
        if (tm_ms <= 0) 
        {
            if (pthread->run) pthread_cancel(pthread->pid);
            break;
        }
    }

    pthread->st = THREAD_OVER; 
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
 * @brief create_thread 
 *
 * @return pointer to new thread, if succ; NULL, if failed
 */
struct thread * create_thread()
{
    struct thread *new_thread = NULL;

    if ((new_thread = (struct thread *)malloc(sizeof(struct thread))) == NULL)
        return NULL;

    memset(new_thread, 0, sizeof(struct thread));
    new_thread->active      = 1;
    new_thread->st          = THREAD_CREATING;
    new_thread->id          = thread_id++;
    new_thread->create_time = get_localtime();

    pthread_mutex_init(&new_thread->lock, NULL);
    pthread_cond_init(&new_thread->ready, NULL);

    return new_thread;
}

/**
 * @brief thread_destroy 
 *
 * @param pthread
 */
void thread_destroy(struct thread *pthread)
{
    if (pthread == NULL) return;

    pthread_mutex_destroy(&pthread->lock);
    pthread_cond_destroy(&pthread->ready);

    free(pthread);
    pthread = NULL;

    return;
}

/**
 * @brief create_pool 
 *
 * @return new pool, if succ; NULL, if failed
 */
struct thread_pool * create_pool()
{
    struct thread_pool *new_pool = NULL;

    if ((new_pool = (struct thread_pool *)malloc(sizeof(struct thread_pool))) == NULL)
        return NULL;

    memset(new_pool, 0, sizeof(struct thread_pool));
    new_pool->active = 1;

    pthread_mutex_init(&new_pool->lock, NULL);
    pthread_cond_init(&new_pool->ready, NULL);

    return new_pool;
}

/**
 * @brief destroy_pool 
 *
 * @param pool
 */
void destroy_pool(struct task_pool *task_pool)
{
    struct thread *pthread = NULL;

    if (task_pool == NULL) {
        return;
    }

    while (task_pool->head != NULL)
    {
        pthread = task_pool->head;
        task_pool->head = task_pool->head->next;
        printf("pool free name: %s\n", pthread->name);
        thread_destroy(pthread);
        pthread = NULL;
    }
}

/**
 * @brief enqueue_pool 
 *
 * @param pool    [in] pthread pool
 * @param pthread [in] pthread
 * 
 * @return 0, if succ; -1, if, failed
 */
int enqueue_pool(struct task_pool *task_pool, struct thread *pthread)
{
    if (task_pool == NULL || pthread == NULL) return -1;

    /**
     * add thread
     */
    if (task_pool->head == NULL) {
        task_pool->head = pthread;
    }
    else task_pool->tail->next = pthread;
    task_pool->tail        = pthread;
    task_pool->tail->next  = NULL;

    return 0;
}

/**
 * @brief dequeue_pool 
 *
 * @param pool
 *
 * @return 
 */
struct thread * dequeue_pool(struct task_pool *task_pool)
{
    struct thread *pthread = NULL;
    if (task_pool == NULL) return NULL;

    pthread = task_pool->head;
    if (pthread != NULL)
        task_pool->head = task_pool->head->next;
    
    return pthread;
}

/**
 * @brief get_pool_size 
 *
 * @param pool
 *
 * @return 
 */
int get_pool_size(struct task_pool *pool)
{
    struct thread *pthread = NULL;
    int size = 0;

    if (pool == NULL) return -1;
    pthread = pool->head;
    while (pthread != NULL) {
        pthread = pthread->next;
        size++;
    }

    return size;
}

/**
 * @brief sig_deal 
 *
 * @param signum [in] number of signal
 */
static void sig_deal(int signum)
{
    switch (signum)
    {
        case SIGKILL:
        case SIGTERM:
        case SIGINT:
        case SIGQUIT:
        case SIGHUP:
        case SIGABRT:
            exit(0);
            break;
        default:
            break;
    }

    return;
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
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGQUIT, &sa, NULL);
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
    if (qthread != NULL) {
        pthread_mutex_lock(&qthread->lock);
        destroy_pool(&qthread->idle_pool);
        destroy_pool(&qthread->run_pool);
        destroy_pool(&qthread->task_pool);
        pthread_mutex_unlock(&qthread->lock);
        
        pthread_mutex_destroy(&qthread->lock);
        pthread_cond_destroy(&qthread->ready);

        free(qthread);
        qthread = NULL;
    }
    if (pool != NULL) {
        pthread_mutex_lock(&pool->lock);
        
        printf("clean pool, idle: %d\n", get_pool_size(&pool->idle_pool));
        destroy_pool(&pool->idle_pool);
        printf("clean run\n");
        destroy_pool(&pool->run_pool);
        printf("clean task\n");
        destroy_pool(&pool->task_pool);
        pthread_mutex_unlock(&pool->lock);
        
        pthread_mutex_destroy(&pool->lock);
        pthread_cond_destroy(&pool->ready);

        free(pool);
        pool = NULL;
    }


    
    return;
}

/**
 * @brief pthread_run 
 *
 * @param thread_id [in] thread id
 */
/*
void pthread_run(int thread_id)
{
    thread_t *pthread = get_pthread(thread_id);

    if (pthread != NULL) {
        pthread->run = 1;
        pthread->st  = THREAD_RUNNING;
    }
}
*/

/**
 * @brief pthread_stop 
 *
 * @param thread_id [in] thread id
 */
/*
void pthread_stop(int thread_id)
{
    thread_t *pthread = get_pthread(thread_id);
    if (pthread != NULL) {
        pthread->run = 0;
        pthread->st  = THREAD_STOPPED;
    }
}
*/
/**
 * @brief pthread_delete
 *
 * @param thread_id [in] thread id
 */
/*
void pthread_delete(int thread_id)
{
    thread_t *pthread = get_pthread(thread_id);

    if (pthread != NULL) {
        pthread->delete = 1;
        pthread->st = THREAD_OVER;
    }
}
*/

/**
 * @brief pthread_hold 
 *
 * @param thread_id [in] thread id
 */
/*
void pthread_hold(int thread_id)
{
    thread_t *pthread = get_pthread(thread_id);

    if (pthread != NULL) {
        pthread->hold = 1;
    }
}
*/

/**
 * @brief pthread_unhold 
 *
 * @param thread_id [in] thread id
 */
/*
void pthread_unhold(int thread_id)
{
    thread_t *pthread = get_pthread(thread_id);

    if (pthread != NULL) {
        pthread->hold = 0;
    }
}
*/

/**
 * @brief pthread_lock 
 *
 * @param thread_id [in] thread id
 */
/*
void pthread_lock(int thread_id)
{
    thread_t *pthread = get_pthread(thread_id);

    if (pthread != NULL) {
        pthread->st = THREAD_LOCK;
        pthread_mutex_lock(&pthread->lock);
    }
}
*/

/**
 * @brief pthread_unlock 
 *
 * @param thread_id [in] thread id
 */
/*
void pthread_unlock(int thread_id)
{
    thread_t *pthread = get_pthread(thread_id);

    if (pthread != NULL) {
        pthread->st = THREAD_RUNNING;
        pthread_mutex_unlock(&pthread->lock);
    }
}
*/

/**
 * @brief pthread_wait 
 *
 * @param thread_id [in] thread id
 */
/*
void pthread_wait(int thread_id)
{
    thread_t *pthread = get_pthread(thread_id);

    if (pthread != NULL && pthread->st == THREAD_LOCK) {
        pthread->st = THREAD_WAIT;
        pthread_cond_wait(&pthread->ready, &pthread->lock);
    }
}
*/

/**
 * @brief pthread_unwait 
 *
 * @param thread_id [in] thread id
 */
/*
void pthread_unwait(int thread_id)
{
    thread_t *pthread = get_pthread(thread_id);

    if (pthread != NULL) {
        pthread->st = THREAD_RUNNING;
        pthread_cond_signal(&pthread->ready);
    }
}
*/

/**
 * @brief pthread_info 
 */
void pthread_info(struct task_pool *task_pool)
{
    thread_t *pthread = NULL;
    const char *state[] = {   
        "idle", 
        "creating", 
        "running", 
        "busy", 
        "stopped", 
        "locked", 
        "waiting", 
        "over"
    };
 
    for (pthread = task_pool->head; pthread != NULL; 
            pthread = pthread->next)
    {
        printf("pool id: %d ---> name: %s ---> status: %s\n", 
                pthread->id, pthread->name, state[pthread->st]);
    }
    /*
    if (qthread != NULL) {
        for (pthread = qthread->idle_pool.head; pthread != NULL; 
                pthread = pthread->next)
        {
            printf("qthread id: %d ---> name: %s ---> status: %s\n", 
                    pthread->id, pthread->name, state[pthread->st]);
        }
    }

    if (pool != NULL) {
        for (pthread = pool->idle_pool.head; pthread != NULL; 
                pthread = pthread->next)
        {
            printf("pool id: %d ---> name: %s ---> status: %s\n", 
                    pthread->id, pthread->name, state[pthread->st]);
        }
    }
    */
}

/**
 * @brief get_idle_thread 
 *
 * @return thread, if succ; NULL, if failed
 */
/*
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
*/

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
    struct thread *task_thread = NULL;
    struct thread *head_thread = NULL;
    int use_time = 0, free_time = 5;
    int idle_cnt = 0;

    while (pool->active) 
    {
        /**
         * execute thread tasks:
         * (1) create new thread when prepared thread all busy, if total
         *     thread count less than max thread count
         * (2) idle thread execute tasks if there is idle thread;
         */
        while (1)
        {
            /**
             * dequeue a task from task pthread pool. if no task, then no
             * need to excute
             */
            pthread_mutex_lock(&pool->lock);
            task_thread = dequeue_pool(&pool->task_pool);
            if (task_thread == NULL) {
                goto unlock;
            }

            /**
             * (1) dequque a thread from idle thread pool
             * (2) if there is no idle thread any more:
             *     1) create new thread if total thread count less than max count;
             *     2) wait idle thread until there is idle thread;
             * (3) if there is some idle threads, then excute tasks
             */
            pthread = dequeue_pool(&pool->idle_pool);
            if (pthread == NULL) {
                if (pool->thread_total_cnt < pool->thread_max_cnt) {
                    /**
                     * create new thread
                     */
                    task_thread->run = 1; 
                    if (pthread_create(&task_thread->pid, NULL, thread_runtine, task_thread) < 0)
                    {
                        enqueue_pool(&pool->task_pool, task_thread);
                        goto unlock;
                    }

                    /**
                     * execute task, and put this thread to run queue
                     */
                    enqueue_pool(&pool->run_pool, task_thread);
                    pool->thread_total_cnt++;
                } else {
                    /**
                     * there is no any idle thread, enqueue this task to the end of task pool
                     */
                    enqueue_pool(&pool->task_pool, task_thread);
                }
            } else {
                /**
                 * execute task, and put this thread to run queue.
                 * before this, need to transform callback function, create_time and so on
                 */
                memcpy(&pthread->worker, &task_thread->worker, sizeof(task_thread->worker));
                pthread->create_time = task_thread->create_time;
                pthread->run = 1;
                enqueue_pool(&pool->run_pool, pthread);
                thread_destroy(task_thread);
            }

unlock:
            pthread_mutex_unlock(&pool->lock);
            usleep(1);
            break;
        }

        usleep(10);
        /**
         * free thread when thread has no any task
         */
        head_thread = NULL;
        while (1)
        {
            /**
             * if idle thread count less than mini count,
             * then no need to free.
             */
            pthread_mutex_lock(&pool->lock);
            idle_cnt = get_pool_size(&pool->idle_pool);
            pthread_mutex_unlock(&pool->lock);
            if (idle_cnt <= pool->thread_mini_cnt) {
                break;
            }

            /**
             * dequeue idle thread:
             * (1) if no idle thread, stop free;
             * (2) if free thread a cycly, then break off while;
             */
            pthread_mutex_lock(&pool->lock);
            pthread = dequeue_pool(&pool->idle_pool);
            pthread_mutex_unlock(&pool->lock);
            if (pthread == NULL) break;
            if (pthread != NULL && pthread == head_thread) {
                pthread_mutex_lock(&pool->lock);
                enqueue_pool(&pool->idle_pool, pthread);
                pthread_mutex_unlock(&pool->lock);
                break;
            }

            /**
             * if idle thread time out, free it; or add it to 
             * the end of idle thread queue.
             */
            pthread_mutex_lock(&pool->lock);
            use_time = get_localtime() - pthread->create_time;
            if (use_time < free_time) {
                enqueue_pool(&pool->idle_pool, pthread);
                if (head_thread == NULL) {
                    head_thread = pthread;
                }
            } else {
                thread_destroy(pthread);
                pool->thread_total_cnt--;
            }
            pthread_mutex_unlock(&pool->lock);
        }

    }

    return NULL;
}

void * free_thread(void *arg)
{
    struct thread *pthread = NULL;

    while (pool->active) 
    {
        pthread_mutex_lock(&pool->lock);
        pthread = dequeue_pool(&pool->run_pool);

        if (pthread == NULL) goto unlock;
        if (pthread->run == 0) {
            enqueue_pool(&pool->idle_pool, pthread);
        } else {
            enqueue_pool(&pool->run_pool, pthread);
        }
unlock:
        pthread_mutex_unlock(&pool->lock);
        usleep(10);
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
    pool = create_pool();
    if (pool == NULL) return -1;
    pool->thread_max_cnt = max_cnt;
    pool->thread_mini_cnt = mini_cnt;

    /**
     * create thread of pool manager
     */
    pthread_create(&pool->pid, NULL, pthread_pool_runtine, pool);
    pthread_create(&pool->pid, NULL, free_thread, pool);
    sem_init(&pool->sem, 0, 0);
    
    /**
     * create thread for pthread pool
     */
    while (init_cnt-- > 0)
    {
        pthread = create_thread();
        if (pthread == NULL) continue;
        pthread->hold = 1;

        if (pthread_create(&pthread->pid, NULL, thread_runtine, pthread) < 0)
        {
            free(pthread);
            pthread = NULL;
            continue;
        }

        pthread_mutex_lock(&pool->lock);
        enqueue_pool(&pool->idle_pool, pthread);
        pool->thread_total_cnt++;
        pthread_mutex_unlock(&pool->lock);
    }

    /**  
     * pthread clean up
     */
    thread_cleanup_event_add();
    atexit(exit_cleanup);

    return 0;
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

    pthread_mutex_lock(&pool->lock);
    pthread = dequeue_pool(&pool->idle_pool);
    pthread_mutex_unlock(&pool->lock);

    if (pthread == NULL) 
    {
        pthread = create_thread();
        if (pthread == NULL) {
            return -1;
        } 
        pthread->hold = 1;
        pthread->worker.handler = handler;
        pthread->worker.arg = arg;

        pthread_mutex_lock(&pool->lock);
        enqueue_pool(&pool->task_pool, pthread);
        pthread_mutex_unlock(&pool->lock);
    }
    else {
        pthread->worker.handler = handler;
        pthread->worker.arg = arg;
        pthread->run = 1;

        pthread_mutex_lock(&pool->lock);
        enqueue_pool(&pool->run_pool, pthread);
        pthread_mutex_unlock(&pool->lock);

    }
    pthread_mutex_unlock(&pool->lock);

    return 0;
}
