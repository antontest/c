#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <utils/utils.h>
#include "mutex.h"

typedef struct private_mutex_t private_mutex_t;
typedef struct private_r_mutex_t private_r_mutex_t;

/**
 * private data of mutex
 */
struct private_mutex_t {

	/**
	 * public functions
	 */
	mutex_t public;

	/**
	 * wrapped pthread mutex
	 */
	pthread_mutex_t mutex;
};

/**
 * @brief lock threading
 */
METHOD(mutex_t, lock, void, private_mutex_t *this)
{
	int err;

	err = pthread_mutex_lock(&this->mutex);
	if (err)
	{
		fprintf(stdout, "!!! MUTEX LOCK ERROR: %s !!!", strerror(err));
	}
}

/**
 * @brief unlock threading
 */
METHOD(mutex_t, unlock, void, private_mutex_t *this)
{
	int err;

	err = pthread_mutex_unlock(&this->mutex);
	if (err)
	{
		fprintf(stdout, "!!! MUTEX UNLOCK ERROR: %s !!!", strerror(err));
	}
}

/**
 * @brief destroy threading
 */
METHOD(mutex_t, mutex_destroy_, void, private_mutex_t *this)
{
	pthread_mutex_destroy(&this->mutex);
	free(this);
}

/**
 * @brief create a thread mutex/lock
 *
 * @return struct mutex
 */
mutex_t *mutex_create()
{
    private_mutex_t *this;

    INIT(this,
        .public = {
            .lock = _lock,
            .unlock = _unlock,
            .destroy = _mutex_destroy_,
        },
    );

    pthread_mutex_init(&this->mutex, NULL);
    return &this->public;
}

/**
 * @brief destroy mutex and free memory
 *
 * @param mutex
 */
void mutex_destroy(mutex_t *mutex)
{
    if (mutex == NULL) return;
    mutex->destroy(mutex);
    free(mutex);
}
