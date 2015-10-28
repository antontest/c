#ifndef __COND_H__
#define __COND_H__
#include "mutex.h"
#include <stdbool.h>
#include <sys/time.h>

typedef struct cond_t cond_t;

/**
 * Condvar wrapper to use in conjunction with mutex_t.
 */
struct cond_t {

	/**
	 * Wait on a condvar until it gets signalized.
	 *
	 * @param mutex			mutex to release while waiting
	 */
	void (*wait)(cond_t *this, mutex_t *mutex);

	/**
	 * Wait on a condvar until it gets signalized, or times out.
	 *
	 * @param mutex			mutex to release while waiting
	 * @param timeout		timeout im ms
	 * @return				TRUE if timed out, FALSE otherwise
	 */
	bool (*timed_wait)(cond_t *this, mutex_t *mutex, unsigned int timeout);

	/**
	 * Wait on a condvar until it gets signalized, or times out.
	 *
	 * The passed timeval should be calculated based on the time_monotonic()
	 * function.
	 *
	 * @param mutex			mutex to release while waiting
	 * @param tv			absolute time until timeout
	 * @return				TRUE if timed out, FALSE otherwise
	 */
	bool (*timed_wait_abs)(cond_t *this, mutex_t *mutex, struct timeval tv);

	/**
	 * Wake up a single thread in a condvar.
	 */
	void (*signal)(cond_t *this);

	/**
	 * Wake up all threads in a condvar.
	 */
	void (*broadcast)(cond_t *this);

	/**
	 * Destroy a condvar and free its resources.
	 */
	void (*destroy)(cond_t *this);
};

/**
 * Create a condvar instance.
 *
 * @param type		type of condvar to create
 * @return			condvar instance
 */
cond_t *cond_create();

/**
 * @brief destroy cond and free memory
 *
 * @param cond
 */
void cond_destroy(cond_t *cond);

#endif /** THREADING_CONDVAR_H_ @} */

