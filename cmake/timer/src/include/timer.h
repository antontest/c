#ifndef __LIBSC_TIMER_H__
#define __LIBSC_TIMER_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <sys/sysinfo.h>
#include <sys/errno.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <getopt.h>
#include <time.h>

#define TIMER_ACCURACY 100

/**
 * \breif Repeasent a tiemr.
 *
 * This structure represent a timer you created, you use the instance of this
 * structure to operate with the timer.\n
 * Don't create the structure yourself, use the libsc_timer_create instead.\n
 * Don't delete the structure yourself, use the libsc_timer_delete instead.\n
 */
typedef struct timer {
    /**
     * This field is used for user to store temp data in a timer. When timerout,
     * the timer will give as a parameter when the callback been called. So you 
     * get the data in callback.
     */
    union {
        int i;
        char c;
        void *p;
    };

    /**
     * Implement releated private field, don't touch it.
     */
    struct timer_impl *impl;
}timer;

/**
 * \brief Timer callback function.
 *
 * You specify a function with the same signature as this function when
 * create the timer. When timeout, the function you specified will be called
 * with parameter tell you which timer is timeout, so you could specify the same
 * callback function with many timers to easy coding.
 *
 * \param timer [in] Parameter tell you which timer is timerout.
 */
typedef void (*timer_cb)(void *arg);

/**
 * \brief Implement releated private field, don't touch it.
 */
typedef struct timer_impl {
    /**
     * activation state
     */
    int active;
    
    /**
     * whether the timer is running cycly
     */
    int repeat;
    
    /**
     * end timer
     */
    int delete;

    /**
     * timer cycle time every time
     */
    int time_r;
    
    /**
     * timer time
     */
    int time_b;
    
    /**
     * callback function of timer
     */
    timer_cb cb;
    void *arg;
    
    /**
     * thread id of timer
     */
    pthread_t thread;
}timer_impl_t;

/**
 * \brief Create a timer.
 *
 * \param tm_ms  [in] Timeout time in microseconds.
 * \param cb     [in] The callback function to call when timeout.This parameter
 *                    could be NULL, but not recommended.
 * \param repeat [in] Specify whether the timer need repeat timeout.
 */
struct timer * timer_creat(int tm_ms, timer_cb cb, void *arg, bool repeat);

/**
 * \brief Start a timer
 *
 * \param timer [in] The timer to start.
 */
void timer_start(struct timer *timer);

/**
 * @brief Stop a timer.
 *
 * @param timer [in] The timer to stop
 */
void timer_stop(struct timer *timer);

/**
 * @brief Delete a timer.
 *
 * @param timer [in] The timer to delete.
 */
void timer_destroy(struct timer *timer);

/**
 * @brief Reset a timer.
 *
 * @param timer [in] The timer to reset.
 */
void timer_reset(struct timer *timer);


#endif
