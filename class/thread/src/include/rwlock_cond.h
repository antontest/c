/**
 * @defgroup rwlock_condvar rwlock_condvar
 * @{ @ingroup threading
 */

#ifndef __RWLOCK_CONDVAR_H__
#define __RWLOCK_CONDVAR_H__

typedef struct rwlock_cond_t rwlock_cond_t;

#include "rwlock.h"

/**
 * A special condvar implementation that can be used in conjunction
 * with rwlock_t (the write lock to be precise).
 *
 * @note The implementation does not verify that the current thread actually
 * holds the write lock and not the read lock, so watch out.
 */
struct rwlock_cond_t {

	/**
	 * Wait on a condvar until it gets signalized.
	 *
	 * @param lock			lock to release while waiting (write lock)
	 */
	void (*wait)(rwlock_cond_t *this, rwlock_t *lock);

	/**
	 * Wait on a condvar until it gets signalized, or times out.
	 *
	 * @param lock			lock to release while waiting (write lock)
	 * @param timeout		timeout im ms
	 * @return				TRUE if timed out, FALSE otherwise
	 */
	bool (*timed_wait)(rwlock_cond_t *this, rwlock_t *lock, u_int timeout);

	/**
	 * Wait on a condvar until it gets signalized, or times out.
	 *
	 * The passed timeval should be calculated based on the time_monotonic()
	 * function.
	 *
	 * @param lock			lock to release while waiting (write lock)
	 * @param tv			absolute time until timeout
	 * @return				TRUE if timed out, FALSE otherwise
	 */
	bool (*timed_wait_abs)(rwlock_cond_t *this, rwlock_t *lock,
						   struct timeval tv);

	/**
	 * Wake up a single thread in a condvar.
	 */
	void (*signal)(rwlock_cond_t *this);

	/**
	 * Wake up all threads in a condvar.
	 */
	void (*broadcast)(rwlock_cond_t *this);

	/**
	 * Destroy a condvar and free its resources.
	 */
	void (*destroy)(rwlock_cond_t *this);
};

/**
 * Create a condvar instance.
 *
 * @return			condvar instance
 */
rwlock_cond_t *rwlock_cond_create();

#endif /** RWLOCK_CONDVAR_H_ @} */
