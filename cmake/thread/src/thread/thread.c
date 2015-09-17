#include <thread.h>

/**
 * @brief pthread runtine
 *
 * @param arg [in] struct pthread_impl pointer
 *
 * @return 
 */
void *thread_runtine(void *arg)
{
    thread_t *impl = (thread_t *)arg;
    pthread_worker_t *worker = &impl->worker;

    while (impl->active) 
    {
        if (!impl->run) continue;

        if (worker->runtine != NULL && impl->run)
        {
            worker->runtine(worker->arg);

            worker->runtine = NULL;
            worker->arg = NULL;
            impl->done = 1;
        }

        if (!impl->keep) break;
        if (impl->delete) break;
    }

    if (impl->on_exit.runtine != NULL) impl->on_exit.runtine(arg);
    impl->active = 0;
    impl->delete = 1;
    impl->done = 1;
    impl->run = 0;

    pthread_mutex_destroy(&impl->lock);
    pthread_cond_destroy(&impl->ready);

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
int thread_start(thread_t *impl, pthread_runtine runtine, void *arg,\
        int run, int keep)
{
    if (impl == NULL || impl->active) return -1;

    impl->worker.runtine = runtine;
    impl->worker.arg = arg;

    impl->run = run;
    impl->keep = keep;
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
}

/**
 * @brief pthread exec another function
 *
 * @param impl [in]
 * @param pr   [in] callback
 * @param arg  [in]
 */
void thread_exec(thread_t *impl, pthread_runtine runtine, void *arg)
{
    if (impl == NULL || !impl->active || !impl->keep) return;

    while (!impl->done) usleep(10);
    impl->run = 0;
    impl->worker.runtine = runtine;
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
void thread_onexit(thread_t *impl, pthread_runtine runtine, void *arg)
{
    if (impl == NULL || !impl->active || !impl->keep) return;

    impl->on_exit.runtine = runtine;
    impl->on_exit.arg = arg;

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

    return;
}
