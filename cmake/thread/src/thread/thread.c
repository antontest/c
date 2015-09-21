#include "thread.h"

static int thread_id = 1;
static int cleanup_event_init = 0;

/**
 * @brief cleanup_runtine -- happened thread be canceled 
 *
 * @param arg
 */
void cleanup_runtine(void *arg)
{
    thread_t *pthread = NULL;
    if (arg == NULL) return;

    /**
     * default thread cancel dealing
     */
    pthread = (thread_t *)arg;
    if (pthread->clean.handler != NULL)
        pthread->clean.handler(pthread->clean.arg);
    pthread->run = 0;
    pthread->delete = 1;
    pthread->state = THREAD_OVER;
    //printf("thread %d be canceled, start to clean\n", pthread->id);

    return;
}

/**
 * @brief pthread handler
 *
 * @param arg [in] struct pthread pointer
 *
 * @return 
 */
void *thread_runtine(void *arg)
{
    /**
     * thread parameter init
     */
    thread_t *pthread = NULL;
    if (arg == NULL) return NULL;
    pthread = (thread_t *)arg;
    thread_worker_t *worker = &pthread->worker;
    pthread->state = THREAD_RUNNING;

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
    while (pthread->active) 
    {
        if (pthread->delete) break;
        if (!pthread->run) continue;

        /**
         * pthread task handle
         */
        if (worker->handler != NULL) {
            /**
             * start execute task, update thread state
             */
            pthread->state = THREAD_BUSY;
            worker->handler(worker->arg);

            /**
             * clear callback, if no repeat
             * update thread state
             */
            if (!pthread->repeat) {
                worker->arg = NULL;
                worker->handler = NULL;
                
                /**
                 * update thread state
                 */
                pthread->run = 0;
                pthread->state = THREAD_IDLE;
            }
        }

        if (!pthread->hold) break;
    }

    //printf("thread %d over\n", pthread->id);
    if (pthread->clean.handler != NULL) 
        pthread->clean.handler(pthread->clean.arg);
    pthread->run = 0;
    pthread->delete = 1;
    pthread->state = THREAD_OVER;
    pthread_cleanup_pop(0);

    return NULL;
}


/******************************************************
**************  Pthread Queue Function  ***************
*******************************************************/

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
    new_thread->state          = THREAD_CREATING;
    new_thread->id          = thread_id++;
    new_thread->create_time = get_localtime();

    //pthread_mutex_init(&new_thread->lock, NULL);
    //pthread_cond_init(&new_thread->ready, NULL);

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

    //pthread_mutex_destroy(&pthread->lock);
    //pthread_cond_destroy(&pthread->ready);

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


/******************************************************
*************** Pthread Clear Function ****************
******************************************************/
/**
 * @brief sig_deal 
 *
 * @param signum [in] number of signal
 */
static void sig_deal(int signum)
{
    printf("cat\n");
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

/**
 * @brief abnormal_event_add 
 */
static void abnormal_event_add()
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
        /**
         * stop main thread
         */
        qthread->active = 0;
        usleep(100);

        /**
         * free memory of thread pool
         */
        pthread_mutex_lock(&qthread->lock);
        printf("qthread clean idle\n");
        destroy_pool(&qthread->idle_pool);

        printf("qthread clean run\n");
        destroy_pool(&qthread->run_pool);

        printf("qthread clean task\n");
        destroy_pool(&qthread->task_pool);
        pthread_mutex_unlock(&qthread->lock);
        
        pthread_mutex_destroy(&qthread->lock);
        pthread_cond_destroy(&qthread->ready);
        sem_destroy(&qthread->sem);

        free(qthread);
        qthread = NULL;
    }
    if (pool != NULL) {
        /**
         * stop main thread
         */
        pool->active = 0;
        usleep(100);

        /**
         * free memory of thread pool
         */
        pthread_mutex_lock(&pool->lock);
        printf("pool clean idle\n");
        destroy_pool(&pool->idle_pool);

        printf("pool clean run\n");
        destroy_pool(&pool->run_pool);

        printf("pool clean task\n");
        destroy_pool(&pool->task_pool);
        pthread_mutex_unlock(&pool->lock);
        
        pthread_mutex_destroy(&pool->lock);
        pthread_cond_destroy(&pool->ready);
        sem_destroy(&pool->sem);

        free(pool);
        pool = NULL;
    }
}


/******************************************************
*************** Pthread Basic Function ****************
******************************************************/
static void * pstart_runtine(void *arg)
{
    struct thread *pthread = NULL;
    
    while (qthread->active)
    {
        plock(&qthread->lock);
        pthread = dequeue_pool(&qthread->run_pool);
        punlock(&qthread->lock);
        if (pthread == NULL) goto wait;
        if (pthread->delete) {
            free(pthread);
        }
        else {
            plock(&qthread->lock);
            enqueue_pool(&qthread->run_pool, pthread);
            punlock(&qthread->lock);
        }

wait:
        usleep(10);
    }

    //0printf("pstart runtine over\n");
    return NULL;
}

/**
 * @brief pstart -- start a thread
 *
 * @param handler [in] callback
 * @param arg     [in] arg
 *
 * @return pid of thread, if succ; -1, if failed
 */
pthread_t pstart(thread_handler handler, void *arg)
{
    struct thread *pthread = NULL;

    /**
     * init pthred pool
     */
    if (qthread == NULL)
        qthread = create_pool();
    if (qthread == NULL) return -1;

    if (handler == NULL) return -1;
    if ((pthread = create_thread()) == NULL)
        return -1;

    pthread->worker.handler = handler;
    pthread->worker.arg = arg;
    pthread->run    = 1;

    if (pthread_create(&pthread->pid, NULL, thread_runtine, pthread) < 0)
    {
        thread_destroy(pthread);
        return -1;
    }
    
    /**
     * create thread of pool manager
     */
    if (qthread->pid <= 0)
    {
        qthread->active = 1;
        if (pthread_create(&qthread->pid, NULL, pstart_runtine, qthread) < 0)
        {
            thread_destroy(pthread);
            return -1;
        }

        /**  
         * pthread clean up
         */
        atexit(exit_cleanup);
        abnormal_event_add();
    }

    plock(&qthread->lock);
    enqueue_pool(&qthread->run_pool, pthread);
    punlock(&qthread->lock);

    return pthread->pid;
} 

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
 * @param mtx [in] pthread mutex
 */
void plock(pthread_mutex_t *mtx)
{
    if (mtx == NULL) return;
    pthread_mutex_lock(mtx);
}

/**
 * @brief ptrylock -- lock thread
 *
 * @param mtx [in] pthread mutex
 */
void ptrylock(pthread_mutex_t *mtx)
{
    if (mtx == NULL) return;
    pthread_mutex_trylock(mtx);
}

/**
 * @brief unlock -- unlock thread
 *
 * @param mtx [in] pthread mutex
 */
void punlock(pthread_mutex_t *mtx)
{
    if (mtx == NULL) return;
    pthread_mutex_unlock(mtx);
}

/**
 * @brief wait -- wait another thread
 *
 * @param cond [in] pthread cond
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
 * @param cond [in] pthread cond
 */
void pcontinue(pthread_cond_t *cond)
{
    if (cond == NULL) return;
    pthread_cond_signal(cond);
}

/**
 * @brief wait pthread over 
 *
 * @param pthread [in]
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
    struct thread *pthread = NULL;
    if (qthread != NULL) {
        pthread_mutex_lock(&qthread->lock);
        qthread->active = 0;
        pthread_mutex_unlock(&qthread->lock);
    }

    if (pool != NULL) {
        while (1) {
            pthread_mutex_lock(&pool->lock);
            if (pool->task_pool.head == NULL) {
                pthread_mutex_unlock(&pool->lock);
                break;
            }
            pthread_mutex_unlock(&pool->lock);
            usleep(10);
        }

        while (1) {
            pthread_mutex_lock(&pool->lock);
            if (pool->run_pool.head == NULL) {
                pthread_mutex_unlock(&pool->lock);
                break;
            }
            pthread_mutex_unlock(&pool->lock);
            usleep(10);
        }
        
        pthread = pool->idle_pool.head;
        while (1) {
            if (pthread == NULL) break;
            pthread_mutex_lock(&pool->lock);
            pthread->delete = 1;
            pthread = pthread->next;
            pthread_mutex_unlock(&pool->lock);
            usleep(10);
        }
         
        pthread_mutex_lock(&pool->lock);
        pool->active = 0;
        pthread_mutex_unlock(&pool->lock);
    }
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
*************** Pthread Attribute Function ************
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
int pthread_start(thread_handler handler, void *arg, int run, int repeat)
{
    struct thread *pthread = NULL;

    /**
     * init thread and thread queue
     */
    if (qthread == NULL) qthread = create_pool();
    if (qthread == NULL) return -1;
    pthread = create_thread();
    if (pthread == NULL) return -1;

    pthread->state = THREAD_CREATING;
    pthread->worker.handler = handler;
    pthread->worker.arg = arg;

    pthread->run    = run;
    pthread->repeat = repeat;
    pthread->active = 1;

    if (pthread_create(&pthread->pid, NULL, thread_runtine, pthread) < 0)
    {
        thread_destroy(pthread);
        return -1;
    }
    enqueue_pool(&qthread->run_pool, pthread);

    /**  
     * pthread clean up
     */
    abnormal_event_add();
    atexit(exit_cleanup);

    return pthread->id;
}

/**
 * @brief get_thread 
 *
 * @param thread_id  [in]  id
 *
 * @return struct thread, if succ; NULL, if failed
 */
struct thread * get_thread(int thread_id)
{
    struct thread *pthread = NULL;
    if (thread_id < 0 || pool == NULL) goto over;
    
    pthread = qthread->run_pool.head;
    while (pthread != NULL)
    {
        if (pthread->id == thread_id) 
            break;
        pthread = pthread->next;
    }

over:
    return pthread;
}

/**
 * @brief let pthread run
 *
 * @param thread id [in]  id
 */
void pthread_run(thread_t *pthread)
{
    if (pthread == NULL || !pthread->active) return;

    pthread->run = 1;
    pthread->state = THREAD_RUNNING;

    return;
}

/**
 * @brief thread_stop 
 *
 * @param pthread [in]
 */
void pthread_stop(thread_t *pthread)
{
    if (pthread == NULL || !pthread->active || !pthread->run) return;
    
    pthread->run = 0;
    pthread->state = THREAD_STOPPED;

    return;
}

/**
 * @brief pthread exec another function
 *
 * @param pthread [in]
 * @param pr   [in] callback
 * @param arg  [in]
 */
void pthread_exec(thread_t *pthread, thread_handler handler, void *arg)
{
    if (pthread == NULL || !pthread->active || !pthread->repeat) return;

    while (pthread->run) usleep(10);
    pthread->run = 0;
    pthread->worker.handler = handler;
    pthread->worker.arg = arg;

    pthread->run = 1;

    return;
}

/**
 * @brief exec function when pthread over
 *
 * @param pthread [in]
 * @param pr   [in] callback
 * @param arg  [in]
 */
void pthread_on_exit(thread_t *pthread, thread_handler handler, void *arg)
{
    if (pthread == NULL || !pthread->active || !pthread->repeat) return;

    pthread->clean.handler = handler;
    pthread->clean.arg     = arg;

    return;
}


/**
 * @brief lock a thread
 * 
 * @param mtx [in] mutex
 *
 * @return 0, if succ; -1, if falied.
 */
/*
int pthread_lock(thread_t *pthread)
{
    if (pthread == NULL || pthread->active) return -1;
    pthread->state = THREAD_LOCK;

    return pthread_mutex_lock(&pthread->lock);
}
*/

/**
 * @brief lock a thread
 * 
 * @param mtx [in] mutex
 *
 * @return 0, if succ; -1, if falied.
 */
/*
int pthread_trylock(thread_t *pthread)
{
    if (pthread == NULL && !pthread->active) return -1;
    pthread->state = THREAD_LOCK;

    return pthread_mutex_trylock(&pthread->lock);
}
*/

/**
 * @brief unlock a thread
 * 
 * @param mtx [in] mutex
 *
 * @return 0, if succ; -1, if falied.
 */
/*
int pthread_unlock(thread_t *pthread)
{
    if (pthread == NULL && !pthread->active) return -1;
    pthread->state = THREAD_RUNNING;

    return pthread_mutex_unlock(&pthread->lock);
}
*/

/**
 * @brief destroy a lock of a thread
 * 
 * @param mtx [in] mutex
 *
 * @return 0, if succ; -1, if falied.
 */
int pthread_delete(thread_t *pthread)
{
    int wait_tm = 10 * 100;
    if (pthread == NULL || !pthread->active) return -1;

    /* wait over for current process */
    if (pthread->run)
    {
        while(wait_tm-- > 0)
        {
            if (!pthread->run) break;
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
            if (!pthread->run) break;
            else usleep(10);
        }
    }

    /* if thread don't end, then call pthread_cancel to end it. */
    if (pthread->run)
    {
        pthread_cancel(pthread->pid);
        //pthread_mutex_destroy(&pthread->lock);
        //pthread_cond_destroy(&pthread->ready);
    }
    pthread->state = THREAD_OVER;

    return 0;
}

/**
 * @brief wait pthread over 
 *
 * @param pthread [in]
 */
void pthread_time_wait_over(thread_t *pthread, int tm_ms)
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

    pthread->state = THREAD_OVER; 
    return;
}



/**
 * @brief pthread_run 
 *
 * @param thread_id [in] thread id
 */

void thread_run(int thread_id)
{
    thread_t *pthread = get_thread(thread_id);

    if (pthread != NULL) {
        pthread->run = 1;
        pthread->state  = THREAD_RUNNING;
    }
}

/**
 * @brief pthread_stop 
 *
 * @param thread_id [in] thread id
 */
void thread_stop(int thread_id)
{
    thread_t *pthread = get_thread(thread_id);
    if (pthread != NULL) {
        pthread->run = 0;
        pthread->state  = THREAD_STOPPED;
    }
}

/**
 * @brief pthread_delete
 *
 * @param thread_id [in] thread id
 */
void thread_delete(int thread_id)
{
    thread_t *pthread = get_thread(thread_id);

    if (pthread != NULL) {
        pthread->delete = 1;
        pthread->state = THREAD_OVER;
    }
}

/**
 * @brief pthread_hold 
 *
 * @param thread_id [in] thread id
 */
void thread_hold(int thread_id)
{
    thread_t *pthread = get_thread(thread_id);

    if (pthread != NULL) {
        pthread->hold = 1;
    }
}

/**
 * @brief pthread_unhold 
 *
 * @param thread_id [in] thread id
 */
void thread_unhold(int thread_id)
{
    thread_t *pthread = get_thread(thread_id);

    if (pthread != NULL) {
        pthread->hold = 0;
    }
}

/**
 * @brief pthread_lock 
 *
 * @param thread_id [in] thread id
 */
/*
void thread_lock(int thread_id)
{
    thread_t *pthread = get_thread(thread_id);

    if (pthread != NULL) {
        pthread->state = THREAD_LOCK;
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
void thread_unlock(int thread_id)
{
    thread_t *pthread = get_thread(thread_id);

    if (pthread != NULL) {
        pthread->state = THREAD_RUNNING;
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
void thread_wait(int thread_id)
{
    thread_t *pthread = get_thread(thread_id);

    if (pthread != NULL && pthread->state == THREAD_LOCK) {
        pthread->state = THREAD_WAIT;
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
void thread_unwait(int thread_id)
{
    thread_t *pthread = get_thread(thread_id);

    if (pthread != NULL) {
        pthread->state = THREAD_RUNNING;
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
                pthread->id, pthread->name, state[pthread->state]);
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
    struct thread *task_thread = NULL;
    struct thread *head_thread = NULL;
    int use_time = 0, free_time = pool->free_time;
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
            if (pool->free_time < 0) {
                pthread_mutex_lock(&pool->lock);
                enqueue_pool(&pool->idle_pool, pthread);
                pthread_mutex_unlock(&pool->lock);
                continue;
            }
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

    //printf("thread pool runtine over\n");
    return NULL;
}

/**
 * @brief run_to_idle_runtine 
 *
 * @param arg
 *
 * @return 
 */
static void * run_to_idle_runtine(void *arg)
{
    struct thread *pthread = NULL;

    while (pool->active) 
    {
        pthread_mutex_lock(&pool->lock);
        pthread = dequeue_pool(&pool->run_pool);

        if (pthread == NULL) goto unlock;
        if (pthread->run == 0) {
            pthread->create_time = get_localtime();
            enqueue_pool(&pool->idle_pool, pthread);
        } else {
            enqueue_pool(&pool->run_pool, pthread);
        }
unlock:
        pthread_mutex_unlock(&pool->lock);
        usleep(10);
    }
        
    //printf("move to idle thread over\n");
    return NULL;
}

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
int pthread_pool_init(int max_cnt, int mini_cnt, int init_cnt, int tm)
{
    struct thread *pthread = NULL;

    /**
     * init pthred pool
     */
    if (pool != NULL || max_cnt <= 0) return 0;
    pool = create_pool();
    if (pool == NULL) return -1;

    /**
     * init pool info
     */
    if (mini_cnt < 0) mini_cnt = 0;
    if (init_cnt < 0) init_cnt = 0;
    if (init_cnt > max_cnt) init_cnt = max_cnt;
    if (mini_cnt > max_cnt) mini_cnt = max_cnt;
    pool->thread_max_cnt = max_cnt;
    pool->thread_mini_cnt = mini_cnt;
    if (tm == 0) tm = DFT_THREAD_TIMEOUT;
    pool->free_time = tm;

    /**
     * create thread of pool manager
     */
    pthread_create(&pool->pid, NULL, pthread_pool_runtine, pool);
    pthread_create(&pool->pid, NULL, run_to_idle_runtine, pool);
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
    abnormal_event_add();
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

    /**
     * dequeue a idle thread:
     * (1) if has, then exec directly;
     * (2) if not, create a new task;
     */
    pthread_mutex_lock(&pool->lock);
    pthread = dequeue_pool(&pool->idle_pool);
    pthread_mutex_unlock(&pool->lock);

    /**
     * create a new task
     */
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
    /**
     * exec task directly
     */
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
