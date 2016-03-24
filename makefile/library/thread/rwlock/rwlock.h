/**
 * @defgroup rwlock rwlock
 * @{ @ingroup threading
 */

#ifndef __THREAD_RWLOCK_H__
#define __THREAD_RWLOCK_H__

typedef struct rwlock_t rwlock_t;

/**
 * Read-Write lock wrapper.
 */
struct rwlock_t {

	/**
	 * Acquire the read lock.
	 */
	void (*read_lock)(rwlock_t *this);

	/**
	 * Acquire the write lock.
	 */
	void (*write_lock)(rwlock_t *this);

	/**
	 * Try to acquire the write lock.
	 *
	 * Never blocks, but returns FALSE if the lock was already occupied.
	 *
	 * @return		TRUE if lock acquired
	 */
	bool (*try_write_lock)(rwlock_t *this);

	/**
	 * Release any acquired lock.
	 */
	void (*unlock)(rwlock_t *this);

	/**
	 * Destroy the read-write lock.
	 */
	void (*destroy)(rwlock_t *this);
};

/**
 * Create a read-write lock instance.
 *
 * @param type		type of rwlock to create
 * @return			unlocked rwlock instance
 */
rwlock_t *rwlock_create();

#endif /** THREADING_RWLOCK_H_ @} */

