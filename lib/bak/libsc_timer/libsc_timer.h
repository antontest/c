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
 */
struct libsc_timer {
    /**
     * This field is used for user to store temp data in a timer.
     */
    union {
        int i;
        char c;
        void *p;
    };

    /**
     * Implement releated private field, don't touch it.
     */
    struct libsc_timer_impl *impl;
};

/**
 * \brief Timer callback function.
 *
 * \param timer [in] Parameter tell you which timer is timerout.
 */
typedef void (*timer_cb)(struct libsc_timer *tm);

struct libsc_timer_impl {
    int active;
    int repeat;
    int delete;
    int time_r;
    int time_b;
    timer_cb cb;
    pthread_t thread;
};

/**
 * \brief Create a timer.
 *
 * \param tm_ms  [in] Timeout time in microseconds.
 * \param cb     [in] The callback function to call when timeout.This parameter
 *                    could be NULL, but not recommended.
 * \param repeat [in] Specify whether the timer need repeat timeout.
 */
struct libsc_timer * libsc_timer_create(int tm_ms, timer_cb cb, bool repeat);

/**
 * \brief Start a timer
 *
 * \param timer [in] The timer to start.
 */
void libsc_timer_start(struct libsc_timer *timer);

/**
 * @brief Stop a timer.
 *
 * @param timer [in] The timer to stop
 */
void libsc_timer_stop(struct libsc_timer *timer);

/**
 * @brief Delete a timer.
 *
 * @param timer [in] The timer to delete.
 */
void libsc_timer_delete(struct libsc_timer *timer);

/**
 * @brief Reset a timer.
 *
 * @param timer [in] The timer to reset.
 */
void libsc_timer_reset(struct libsc_timer *timer);
#endif
