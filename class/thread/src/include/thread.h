#ifndef __THREAD_H__
#define __THREAD_H__
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

typedef struct thread_t thread_t;

/**
 * Main function of a thread.
 *
 * @param arg			argument provided to constructor
 * @return				value provided to threads joining the terminating thread
 */
typedef void *(*thread_main_t)(void *arg);

/**
 * Cleanup callback function for a thread.
 *
 * @param arg			argument provided to thread_cleanup_push
 */
typedef void (*thread_cleanup_t)(void *arg);


/**
 * Thread wrapper implements simple, portable and advanced thread functions.
 *
 * @note All threads other than the main thread need either to be joined or
 * detached by calling the corresponding method.
 */
struct thread_t {

	/**
	 * Cancel this thread.
	 */
	void (*cancel)(thread_t *this);

	/**
	 * Send a signal to this thread.
	 *
	 * @param sig		the signal to be sent to this thread
	 */
	void (*kill)(thread_t *this, int sig);

	/**
	 * Detach this thread, this automatically destroys the thread object after
	 * the thread returned from its main function.
	 *
	 * @note Calling detach is like calling destroy on other objects.
	 */
	void (*detach)(thread_t *this);

	/**
	 * Join this thread, this automatically destroys the thread object
	 * afterwards.
	 *
	 * @note Calling join is like calling destroy on other objects.
	 *
	 * @return			the value returned from the thread's main function or
	 *					a call to exit.
	 */
	void *(*join)(thread_t *this);

    /**
     * @brief destroy instance and free memory
     */
    void (*destroy) (thread_t *this);

    /**
     * @brief get thread id
     */
    int (*get_id) (thread_t *this);
};


/**
 * Create a new thread instance.
 *
 * @param main			thread main function
 * @param arg			argument provided to the main function
 * @return				thread instance
 */
thread_t *thread_create(thread_main_t main, void *arg);

/**
 * Get a thread object for the current thread.
 *
 * @return				thread instance
 */
thread_t *thread_current();

/**
 * Get the human-readable ID of the current thread.
 *
 * The IDs are assigned incrementally starting from 1.
 *
 * @return				human-readable ID
 */
u_int thread_current_id();

/**
 * Push a function onto the current thread's cleanup handler stack.
 * The callback function is called whenever the thread is cancelled, exits or
 * thread_cleanup_pop is called with TRUE as execute argument.
 *
 * @param cleanup		function called on thread exit
 * @param arg			argument provided to the callback
 */
void thread_cleanup_push(thread_cleanup_t cleanup, void *arg);

/**
 * Remove the top function from the current thread's cleanup handler stack
 * and optionally execute it.
 *
 * @param execute		TRUE to execute the function
 */
void thread_cleanup_pop(bool execute);

/**
 * Enable or disable the cancelability of the current thread. The current
 * value is returned.
 *
 * @param enable		TRUE to enable cancelability
 * @return				the current state of the cancelability
 */
bool thread_cancelability(bool enable);

/**
 * Force creation of a cancellation point in the calling thread.
 */
void thread_cancellation_point();

/**
 * Exit the current thread.
 *
 * @param val			value provided to threads joining the current thread
 */
void thread_exit(void *val);

/**
 * Called by the main thread to initialize the thread management.
 */
void threads_init();

/**
 * Called by the main thread to deinitialize the thread management.
 */
void threads_deinit();


#endif /** THREADING_THREAD_H_ @} */
