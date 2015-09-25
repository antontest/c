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



