#include "thread.h"

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
 * @brief set_joinable
 *   
 *   set thread joinable
 *
 *
 * @return 0, if succ; -1, if failed
 */
int set_joinable(int enable)
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



<<<<<<< HEAD
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
    //int use_time = 0, free_time = pool->free_time;
    //int idle_cnt = 0;

    pool->state = THREAD_BUSY;
    while (pool->active) 
    {
        /**
         * execute thread tasks:
         * (1) create new thread when prepared thread all busy, if total
         *     thread count less than max thread count
         * (2) idle thread execute tasks if there is idle thread;
         */
        pthread_mutex_lock(&pool->lock);
        while (pool->active)
        {
            /**
             * dequeue a task from task pthread pool. if no task, then no
             * need to excute
             */
            task_thread = dequeue_pool(&pool->task_pool);
            if (task_thread == NULL) break;

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
                        jumphead_pool(&pool->task_pool, task_thread);
                        continue;
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
                    jumphead_pool(&pool->task_pool, task_thread);
                    break;
                }
            } else {
                /**
                 * execute task, and put this thread to run queue.
                 * before this, need to transform callback function, create_time and so on
                 */
                //pthread_mutex_lock(&pthread->lock);
                //memcpy(&pthread->worker, &task_thread->worker, sizeof(task_thread->worker));
                //pthread->active = task_thread->active;
                //pthread->pid = task_thread->pid;
                //pthread->id = task_thread->id;
                pthread->worker.handler = task_thread->worker.handler;
                pthread->worker.arg = task_thread->worker.arg;
                pthread->create_time = task_thread->create_time;
                pthread->run = 1;
                //pthread_mutex_unlock(&pthread->lock);
                thread_destroy(task_thread);

                enqueue_pool(&pool->run_pool, pthread);
            }
        }
        pthread_mutex_unlock(&pool->lock);

        /**
         * move idle thread of run thread to idle thread pool
         */
        pthread_mutex_lock(&pool->lock);
        head_thread = NULL;
        while (pool->active) 
        {
            pthread = dequeue_pool(&pool->run_pool);

            if (pthread == NULL) break;
            if (pthread != NULL && pthread == head_thread) {
                enqueue_pool(&pool->run_pool, pthread);
                break;
            }
            if (pthread->run == 0) {
                pthread->create_time = get_localtime();
                enqueue_pool(&pool->idle_pool, pthread);
            } else {
                enqueue_pool(&pool->run_pool, pthread);

                if (head_thread == NULL) head_thread = pthread;
            }
            usleep(1);
        }
        pthread_mutex_unlock(&pool->lock);

        /**
         * pexit call
         */
        if (pool->exit) {
            if (pool->task_pool.head != NULL) continue;
            if (pool->run_pool.head != NULL) continue;
            
            pthread = pool->idle_pool.head;
            while (pthread != NULL) {
                pthread_mutex_lock(&pthread->lock);
                pthread->delete = 1;
                pthread_mutex_unlock(&pthread->lock);
                while (pthread->state != THREAD_OVER) usleep(10);
                pthread = pthread->next;
            }

            break;
        }

    }

    pthread_mutex_unlock(&pool->lock);
    pool->state = THREAD_OVER;
    pool_thread_over_cnt++;
    pthread_mutex_unlock(&pool->lock);
    printf("thread pool runtine over\n");
    return NULL;
}

/**
 * @brief run_to_idle_runtine 
 *
 * @param arg
 *
 * @return 
 */
void * run_to_idle_runtine(void *arg)
{
    struct thread *pthread = NULL;

    while (pool->active) 
    {
        pthread_mutex_lock(&pool->lock);
        pool->state = THREAD_RUNNING;
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
        
    pthread_mutex_lock(&pool->lock);
    pool->state = THREAD_OVER;
    pool_thread_over_cnt++;
    punlock(&pool->lock);
    //printf("move to idle thread over\n");

    return NULL;
}

/**
 * @brief free_idle_runtine 
 *
 * @param arg
 *
 * @return 
 */
static void * free_idle_runtine(void *arg)
{
    struct timeval tv = {0};
    struct thread *pthread = NULL;
    struct thread *pre = NULL;
    int idle_cnt = 0;;

    while (pool->active && !pool->exit) {
        tv.tv_sec = pool->free_time;
        tv.tv_usec = 0;

        pre = NULL;
        select(0, NULL, NULL, NULL, &tv);
        idle_cnt = get_pool_size(&pool->idle_pool) - pool->thread_mini_cnt;
        if (idle_cnt <= 0)
            continue;
        plock(&pool->lock);
        while (idle_cnt-- > 0) {
            pthread = dequeue_pool(&pool->idle_pool);
            pthread_mutex_lock(&pthread->lock);
            pthread->delete = 1;
            pthread_mutex_unlock(&pthread->lock);
            usleep(100);
            if (pthread->state != THREAD_OVER) pcancel(pthread->pid);
            thread_destroy(pthread);
        }
        punlock(&pool->lock);
    }

    plock(&pool->lock);
    pool_thread_over_cnt++;
    punlock(&pool->lock);
    printf("free idle thread over\n");
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
    //pthread_create(&pool->pid, NULL, run_to_idle_runtine, pool);
    pthread_create(&pool->pid, NULL, free_idle_runtine, pool);
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
    //atexit(atexit_handle);

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
        pthread_mutex_lock(&pthread->lock);
        pthread->worker.handler = handler;
        pthread->worker.arg = arg;
        pthread->run = 1;
        pthread_mutex_unlock(&pthread->lock);

        pthread_mutex_lock(&pool->lock);
        enqueue_pool(&pool->run_pool, pthread);
        pthread_mutex_unlock(&pool->lock);

    }
    pthread_mutex_unlock(&pool->lock);

    return 0;
}
=======
>>>>>>> 1c5880672ca03c2cdcdbaf6981fe8a8ae0aa1102
