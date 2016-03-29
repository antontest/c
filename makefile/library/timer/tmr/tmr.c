#include <tmr.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <utils/utils.h>
#include <linked_list.h>
#include <thread.h>
#include <mutex.h>
#include <cond.h>
#include <bsem.h>
#include <sys/time.h>

typedef struct private_tmr_t private_tmr_t;
struct private_tmr_t {
    /**
     * @brief public interface
     */
    tmr_t public;

    /**
     * @brief list of timers
     */
    linked_list_t *timers;

    /**
     * @brief thread of timer
     */
    thread_t *thread;

    /**
     * @brief sem
     */
   
    bsem_t *sem;

    /**
     * @brief control timer thread
     */
    unsigned char stop;
};

typedef struct tmr_lock_t tmr_lock_t;
struct tmr_lock_t {
    tmr_arg_t arg;
    unsigned short run_cnt;
    struct timeval next;
};

static void next_time_point(struct timeval *tm, unsigned short wait)
{
    tm->tv_sec += wait / 1000 + (tm->tv_usec + wait % 1000 * 1000) / 1000000;
    tm->tv_usec = (tm->tv_usec + wait % 1000 * 1000) % 1000000;
}

static tmr_lock_t *tmr_lock_create(unsigned short wait, short event, unsigned short cnt, void (*cb) (void *arg), void *arg)
{
    tmr_lock_t *this; 

    INIT(this,
        .arg = {
            .wait  = wait,
            .event = event,
            .cnt   = cnt,
            .cb    = cb,
            .arg   = arg,
        },
        .next = {
            .tv_sec  = 0,
            .tv_usec = 0,
        },
        .run_cnt = 0,
    );

    gettimeofday(&this->next, NULL);
    this->next.tv_sec += wait / 1000 + (this->next.tv_usec + wait % 1000 * 1000) / 1000000;
    this->next.tv_usec = (this->next.tv_usec + wait % 1000 * 1000) % 1000000;
    next_time_point(&this->next, wait);

    return this;
}

METHOD(tmr_t, start_, int, private_tmr_t *this, tmr_arg_t *arg)
{
    tmr_lock_t *tmr = NULL;

    if (!arg || this->stop) return -1;
    tmr = tmr_lock_create(arg->wait, arg->event, arg->cnt, arg->cb, arg->arg);
    if (!tmr) return -1;

    this->timers->insert_last(this->timers, tmr);
    this->sem->post(this->sem);

    return 0;
}

METHOD(tmr_t, destroy_, void, private_tmr_t *this)
{
    tmr_arg_t *tmr = NULL;

    /**
     * stop thread
     */
    this->stop = 1;
    this->sem->post(this->sem);
    usleep(100);
    this->thread->cancel(this->thread);
    threads_deinit();

    /**
     * free timers
     */
    this->timers->reset_enumerator(this->timers);
    while (this->timers->enumerate(this->timers, (void **)&tmr)) {
        free(tmr);
    }
    this->timers->destroy(this->timers);
    this->sem->destroy(this->sem);
}

static void timers_thread(private_tmr_t *this)
{
    tmr_lock_t *tmr = NULL;
    struct timeval tm;

    while (!this->stop) {
       this->sem->wait(this->sem); 

       while (!this->stop) {
           this->timers->reset_enumerator(this->timers);
           while (this->timers->enumerate(this->timers, (void **)&tmr)) {
               gettimeofday(&tm, NULL);
               if ((tmr->run_cnt < tmr->arg.cnt) &&
                  ((tm.tv_sec > tmr->next.tv_sec || 
                  (tm.tv_sec == tmr->next.tv_sec && 
                   tm.tv_usec > tmr->next.tv_usec)))
                  ) {
                   tmr->arg.cb(tmr->arg.arg);
                   if (++tmr->run_cnt < tmr->arg.cnt)
                       next_time_point(&tmr->next, tmr->arg.wait);
               }
           }
       }
    }
}

tmr_t *tmr_create()
{
    private_tmr_t *this;

    INIT(this, 
        .public = {
            .start   = _start_,
            .destroy = _destroy_,
        },
        .timers = linked_list_create(),
        .sem    = bsem_create(0),
        .stop   = 0,
    );

    threads_init();
    this->thread = thread_create((void *)timers_thread, this);

    return &this->public;
}
