/**
 * @defgroup timer
 * @{ @ingroup timer
 */

#ifndef __timer_H__
#define __timer_H__

typedef enum timer_state_t {
    TIMER_STARTING  = 100,
    TIMER_RUNNING   ,
    TIMER_PAUSED    ,
    TIMER_DESTROYED 
} timer_state_t;

typedef struct timer timer;

/**
 * @brief  function of timer
 *
 * @param  [in] argument provided to timer
 */
typedef void (*timer_main_t)(void *arg);

/**
 * @brief timer to do something
 */
struct timer {
    /**
     * let timer start to run
     */
    void (*start)(timer *this);

    /**
     * pause timer to wait
     */
    void (*pause)(timer *this);

    /**
     * resume timer to run again
     */
    void (*resume)(timer *this);

    /**
     * delete timer and free it
     */
    void (*destroy)(timer *this);

    /**
     * get timer running state
     */
    int (*get_state)(timer *this);

    /**
     * get timer running state's string
     */
    char *(*get_state_str)(timer *this);

    /**
     * get timer running times
     */
    unsigned int (*get_runtimes)(timer *this);

    /**
     * set timer interval
     */
    void (*set_interval)(timer *this, unsigned int interval);
};

/**
 * @brief create a new timer to run cyclely
 *
 * @param main           [in] timer main function
 * @param arg            [in] argument provided to the main function
 * @param timer_interval [in] timer waiting time, ms
 *
 * @return timer instance
 */
timer *timer_start(timer_main_t main, void *arg, unsigned int timer_interval);

#endif /** __TIMER_H__ @} */
