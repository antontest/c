#include <pool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <thread.h>
#include <bsem.h>
#include <mutex.h>
#include <utils/utils.h>
#include <data/linked_list.h>

typedef struct private_pool_t private_pool_t;
struct private_pool_t {
    /**
     * @brief public interface
     */
    pool_t public;

    /**
     * @brief pool whether created succ
     */
    int created;

    /**
     * @brief control manager thread stop
     */
    int stop;

    /**
     * @brief thread pool size
     */
    int size;

    /**
     * @brief thread of managing thread and task
     */
    thread_t *manager;

    /**
     * @brief list of thread
     */
    linked_list_t *thread_list;

    /**
     * @brief task in this pool
     */
    linked_list_t *task_list;

    /**
     * @brief lock of thread list;
     */
    mutex_t *thread_list_lock;

    /**
     * @brief lock of task list;
     */
    mutex_t *task_list_lock;

    /**
     * @brief deal with task add and remove
     */
    bsem_t *has_work;
};
#define pthread_list   this->thread_list
#define ptask_list     this->task_list
#define pool_size      this->size
#define task_lock      this->task_list_lock
#define manager_thread this->manager
#define pool_has_work  this->has_work
#define pthread_lock   this->thread_list_lock

/**
 * @brief thread count in this thread pool
 */
int pool_used_cnt = 0;

/**
 * @brief lock of thread list;
 */
mutex_t *thread_list_lock = NULL;

/**
 * @brief deal with task add and remove
 */
bsem_t *btask = NULL;
#define pool_used    pool_used_cnt
#define thread_lock  thread_list_lock
#define task_bsem    btask
static private_pool_t *pool_ptr = NULL;

typedef struct thread_task_t thread_task_t;
struct thread_task_t {
    void (*work) (void *);
    void *arg;
};

thread_task_t *create_thread_task(void (*work) (void *), void *arg)
{
    thread_task_t *this;

    INIT(this,
        .work = work,
        .arg  = arg,
    );

    return this;
}

typedef enum thread_state_t thread_state_t;
enum thread_state_t {
    THREAD_IDLE = 0,
    THREAD_WORKING
};

typedef struct thread_pkg_t thread_pkg_t;
struct thread_pkg_t {
    /**
     * @brief thread id
     */
    int id;

    /**
     * @brief control thread stop
     */
    int stop;

    /**
     * @brief thread state, idle or working
     */
    thread_state_t state;

    /**
     * @brief thread
     */
    thread_t *thread;

    /**
     * @brief wait_job
     */
    bsem_t *wait_job;

    /**
     * @brief thread task
     */
    thread_task_t *task;

    /**
     * @brief task lock
     */
    mutex_t *lock;
};

thread_pkg_t *create_thread_pkg()
{
    thread_pkg_t *this;

    INIT(this,
        .id     = -1,
        .stop   = 0,
        .wait_job   = NULL,
        .thread = NULL,
    );

    return this;
}

METHOD(pool_t, destroy_, void, private_pool_t *this)
{
    int task_cnt   = 0;
    int thread_cnt = 0;
    thread_pkg_t *thread = NULL;
    thread_task_t *task  = NULL;

    /**
     * destroy manager thread and free memory
     */
    if (manager_thread) {
        this->stop = 1;
        if (task_bsem) task_bsem->post(task_bsem);
        if (pool_has_work) pool_has_work->post(pool_has_work);

        usleep(10);
        manager_thread->cancel(manager_thread);
    }

    /**
     * destroy thread pool
     */
    if (pthread_list) {
        thread_cnt = pthread_list->get_count(pthread_list);
        while (thread_cnt-- > 0) {
            pthread_list->remove_first(pthread_list, (void **)&thread);
            if (!thread) continue;

            thread->stop = 1;
            thread->wait_job->post(thread->wait_job);
            usleep(10);

            thread->thread->cancel(thread->thread);
            thread->wait_job->destroy(thread->wait_job);
            thread->lock->unlock(thread->lock);
            thread->lock->destroy(thread->lock);
            if (thread->task) free(thread->task);
            free(thread);
        }
        free(pthread_list);
    }

    /**
     * free task
     */
    if (ptask_list) {
        task_cnt = ptask_list->get_count(ptask_list);
        while (task_cnt-- > 0) {
            ptask_list->remove_first(ptask_list, (void **)&task);
            if (task) free(task);
        }
        free(ptask_list);
    }

    /**
     * free lock and wait_job
     */
    if (pthread_lock) pthread_lock->destroy(pthread_lock);
    if (task_lock) task_lock->destroy(task_lock);
    if (pool_has_work) pool_has_work->destroy(pool_has_work);
    free(this);

    /**
     * destroy global memory
     */
    if (thread_lock) thread_lock->destroy(thread_lock);
    if (task_bsem) task_bsem->destroy(task_bsem);
}

/**
 * @brief thread handler 
 */
static void thread_handler(thread_pkg_t *this)
{
    while (!this->stop) {
        /**
         * thread waiting for job
         */ 
        this->wait_job->wait(this->wait_job);
        if (this->stop) break;

        /**
         * if has work to do
         */
        if (this->task->work != NULL) {
            thread_lock->lock(thread_lock);
            pool_used++;
            thread_lock->unlock(thread_lock);

            this->lock->lock(this->lock);
            this->state = THREAD_WORKING;
            this->task->work(this->task->arg);

            this->task->work = NULL;
            this->task->arg  = NULL;
            this->lock->unlock(this->lock);

            thread_lock->lock(thread_lock);
            pool_used--;
            task_bsem->post(task_bsem);
            this->state = THREAD_IDLE;
            thread_lock->unlock(thread_lock);
        }
    }
}

/**
 * @brief thread manager handler 
 */
static void thread_manager_handler(private_pool_t *this)
{
    thread_pkg_t *thread = NULL;
    thread_task_t *task  = NULL;

    while (!this->stop) {   
        /**
         * waiting for jobs and idle thread
         */
        pool_has_work->wait(pool_has_work);
        task_bsem->wait(task_bsem);
        if (this->stop) break;

        /**
         * find idle task
         */
        pthread_list->reset_current(pthread_list);
        while (pthread_list->get_count(pthread_list) > 0 && 
               ptask_list->get_count(ptask_list)) {

            /**
             * get one thread, if working, then continue util find idle
             */
            thread_lock->lock(thread_lock);
            pthread_list->get_next(pthread_list, (void **)&thread);
            thread_lock->unlock(thread_lock);
            if (thread->state != THREAD_IDLE) continue;

            /**
             * get one job, let thread execute
             */
            task_lock->lock(task_lock);
            ptask_list->remove_first(ptask_list, (void **)&task);
            task_lock->unlock(task_lock);

            thread->lock->lock(thread->lock);
            memset(thread->task, 0, sizeof(thread_task_t));
            memcpy(thread->task, task, sizeof(thread_task_t));
            free(task);

            usleep(1); /* sleep a while for let thread task take effect */
            thread->lock->unlock(thread->lock);
            thread->wait_job->post(thread->wait_job);
        }
    }
}

/**
 * @brief error handler 
 */
static void error_handler(int sig, siginfo_t *info, void *text)
{
    switch (sig) {
        case SIGINT:
        case SIGTERM:
        case SIGKILL:
        case SIGSTOP:
            _destroy_(pool_ptr);
            exit(1);
            break;
    }
}

/**
 * @brief create new thread to thread pool
 */
static int add_one_new_thread_to_pool(private_pool_t *this)
{
    thread_pkg_t *thread = NULL;

    thread = create_thread_pkg();
    if (!thread) return -1;

    thread->task = create_thread_task(NULL, NULL);
    if (!thread->task) return -1;

    thread->wait_job = bsem_create(1);
    if (!thread->wait_job) return -1;
    thread->lock = mutex_create();
    if (!thread->lock) return -1;

    thread->thread = thread_create((void *)thread_handler, thread);
    if (!thread->thread) return -1;
    thread->id = thread->thread->get_id(thread->thread);
    thread->state = THREAD_IDLE;
    pthread_lock->lock(pthread_lock);
    pthread_list->insert_last(pthread_list, thread);
    pthread_lock->unlock(pthread_lock);

    return 0;
}

/**
 * @brief init thread pool 
 */
static int init_pool(private_pool_t *this)
{
    int i = 0;
    struct sigaction act;

    /**
     * act signal handler
     */
    act.sa_sigaction = error_handler;
    sigaction(SIGINT, &act, NULL);
    sigaction(SIGTERM, &act, NULL);
    sigaction(SIGKILL, &act, NULL);
    sigaction(SIGSTOP, &act, NULL);

    /**
     * 1. create manager thread
     * 2. create wait_job
     * 3. create thread lock
     */
    threads_init();
    task_bsem = bsem_create(pool_size);
    if (!task_bsem) return -1;
    thread_lock = mutex_create();
    if (!thread_lock) return -1;
    manager_thread = thread_create((void *)thread_manager_handler, this);
    if (!manager_thread) return -1;

    /**
     * create thread in pool
     */
    for (i = 0; i < pool_size; i++) {
        if (add_one_new_thread_to_pool(this) < 0) break;
    }
    if (i < pool_size) return -1;
    this->created = 1;

    return 0;
}

METHOD(pool_t, addjob_, int, private_pool_t *this, void (*job) (void *), void *arg)
{
    thread_task_t *task = NULL;

    while (!this->created) usleep(10);
    if (!ptask_list) return -1;
        
    task = create_thread_task(job, arg);
    if (!task) return -1;

    task_lock->lock(task_lock);
    ptask_list->insert_last(ptask_list, task);
    pool_has_work->post(pool_has_work);
    task_lock->unlock(task_lock);

    return 0;
}

pool_t *create_pool(int size)
{
    private_pool_t *this;

    if (pool_ptr) return &pool_ptr->public;
    INIT(this,
        .public = {
            .addjob  = _addjob_,
            .destroy = _destroy_,
        },
        .created  = 0,
        .stop     = 0, 
        .size     = size,
        .manager  = NULL,
        .has_work = bsem_create(0),
        .thread_list = linked_list_create(),
        .task_list   = linked_list_create(),
        .thread_list_lock = mutex_create(),
        .task_list_lock   = mutex_create(),
    );

    if (init_pool(this) != 0) {
        _destroy_(this);
        return NULL;
    }

    pool_ptr = this;
    return &this->public;
}
