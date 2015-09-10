#include <timer.h>

/**
 * \brief thread function of thread.
 *
 * \param timer  [in] parameter of thread.
 */
void *timer_routine(void *timer)
{
    struct timeval tv = {0};
    struct timer_impl *impl = NULL;
    
    if (timer == NULL) return NULL;
    impl = ((struct timer *)timer)->impl;

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
struct timer * timer_creat(int tm_ms, timer_cb cb, bool repeat)
{
   struct timer *timer = NULL;
   struct timer_impl *impl = NULL;
   int ret = 0;

   impl =  (struct timer_impl *)malloc(sizeof(struct timer_impl));
   if (impl == NULL) goto TIMER_CREATE_FAIL;

   timer =  (struct timer *)malloc(sizeof(struct timer));
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
void timer_start(struct timer *timer)
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
void timer_stop(struct timer *timer)
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
void timer_destroy(struct timer *timer)
{
    if (timer == NULL) return;

    timer->impl->delete = 1;
}

/**
 * @brief Reset a timer.
 *
 * @param timer [in] The timer to reset.
 */
void timer_reset(struct timer *timer)
{
    if (timer == NULL) return;

    timer->impl->time_r = timer->impl->time_b;
    
    return ;
}

