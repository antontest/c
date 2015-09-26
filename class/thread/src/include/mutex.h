#ifndef __MUTEX_H__
#define __MUTEX_H__
#include "utils/utils.h"

typedef struct mutex_t mutex_t;


/**
 * Mutex wrapper implements simple, portable and advanced mutex functions.
 */
struct mutex_t {

	/**
	 * Acquire the lock to the mutex.
	 */
	void (*lock)(mutex_t *this);

	/**
	 * Release the lock on the mutex.
	 */
	void (*unlock)(mutex_t *this);

	/**
	 * Destroy a mutex instance.
	 */
	void (*destroy)(mutex_t *this);
};

/**
 * Create a mutex instance.
 *
 * @return	unlocked mutex instance
 */
mutex_t *mutex_create();

/**
 * @brief destroy mutex and free memory
 *
 * @param mutex
 */
void mutex_destroy(mutex_t *mutex);

#endif /** THREADING_MUTEX_H_ @} */

