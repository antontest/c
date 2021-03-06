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
struct libsc_timer {
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
    struct libsc_timer_impl *impl;
};

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

void *timer_routine(void *timer)
{
    struct timeval tv = {0};
    struct libsc_timer_impl *impl = NULL;
    
    if (timer == NULL) return NULL;
    impl = ((struct libsc_timer *)timer)->impl;

    while (1) {
        tv.tv_sec = 0;
        tv.tv_usec = 1000 * TIMER_ACCURACY;

        select(0, NULL, NULL, NULL, &tv);
        if (impl->delete) break;
        if (!impl->active) continue;

        impl->time_r -= TIMER_ACCURACY;
        if (impl->time_r <= 0) {
            if (impl->repeat) impl->time_r = impl->time_b;
            else impl->delete = 1;

            impl->cb(timer);
        }
    }

    if (impl != NULL) free(impl);
    if (timer != NULL) free(timer);
    impl = NULL;
    timer = NULL;

    return NULL;
}

/**
 * \brief Create a timer.
 *
 * \param tm_ms  [in] Timeout time in microseconds.
 * \param cb     [in] The callback function to call when timeout.This parameter
 *                    could be NULL, but not recommended.
 * \param repeat [in] Specify whether the timer need repeat timeout.
 */
struct libsc_timer * libsc_timer_create(int tm_ms, timer_cb cb, bool repeat)
{
   struct libsc_timer *timer = NULL;
   struct libsc_timer_impl *impl = NULL;
   int ret = 0;

   impl =  (struct libsc_timer_impl *)malloc(sizeof(struct libsc_timer_impl));
   if (impl == NULL) goto TIMER_CREATE_FAIL;

   timer =  (struct libsc_timer *)malloc(sizeof(struct libsc_timer));
   if (timer == NULL) goto TIMER_CREATE_FAIL;

   impl->active = 0;
   impl->repeat = repeat;
   impl->delete = 0;
   impl->time_b = tm_ms;
   impl->time_r = tm_ms;
   impl->cb = cb;
   timer->impl = impl;

   ret = pthread_create(&impl->thread, NULL, timer_routine, timer);
   if (ret != 0) goto TIMER_CREATE_FAIL;
   pthread_detach(impl->thread);

   return timer;

TIMER_CREATE_FAIL:
    if (impl != NULL) free(impl);
    if (timer != NULL) free(timer);
    impl = NULL;
    timer = NULL;
    
    return NULL;
}

/**
 * \brief Start a timer
 *
 * \param timer [in] The timer to start.
 */
void libsc_timer_start(struct libsc_timer *timer)
{
    if (timer == NULL) return;

    timer->impl->time_r = timer->impl->time_b;
    timer->impl->active = 1;
    
    return ;
}

/**
 * @brief Stop a timer.
 *
 * @param timer [in] The timer to stop
 */
void libsc_timer_stop(struct libsc_timer *timer)
{
    if (timer == NULL) return;

    timer->impl->active = 0;

    return ;
}

/**
 * @brief Delete a timer.
 *
 * @param timer [in] The timer to delete.
 */
void libsc_timer_delete(struct libsc_timer *timer)
{
    if (timer == NULL) return;

    timer->impl->delete = 1;
}

/**
 * @brief Reset a timer.
 *
 * @param timer [in] The timer to reset.
 */
void libsc_timer_reset(struct libsc_timer *timer)
{
    if (timer == NULL) return;

    timer->impl->time_r = timer->impl->time_b;
    
    return ;
}

/*void cb(struct libsc_timer *timer)
{
    printf("%d\n", timer->i++);
    return;
}

int main(int agrc, char *agrv[])
{
    struct libsc_timer *t1 = NULL;

    if ((t1 = libsc_timer_create(1000, cb, 1)) == NULL) {
        printf("create failed.\n");
        return -1;
    }
    t1->i = 1;
    libsc_timer_start(t1);
    sleep(5);
    libsc_timer_stop(t1);
    sleep(5);
    libsc_timer_start(t1);
    sleep(5);
    libsc_timer_delete(t1);
    sleep(1);

    return 0;
}*/

