#include <pthread.h>
#include <stdint.h>
#include <time.h>
#include <errno.h>
#include "utils/utils.h"
#include "cond.h"

typedef struct private_mutex_t private_mutex_t;
typedef struct private_cond_t private_cond_t;

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
 * private data of cond
 */
struct private_cond_t {

	/**
	 * public functions
	 */
	cond_t public;

	/**
	 * wrapped pthread condvar
	 */
	pthread_cond_t cond;

};

METHOD(cond_t, wait_, void, private_cond_t *this, private_mutex_t *mutex)
{
    pthread_cond_wait(&this->cond, &mutex->mutex);
}

/* use the monotonic clock based version of this function if available */
#ifdef HAVE_PTHREAD_COND_TIMEDWAIT_MONOTONIC
#define pthread_cond_timedwait pthread_cond_timedwait_monotonic
#endif

METHOD(cond_t, timed_wait_abs, bool, private_cond_t *this, private_mutex_t *mutex, struct timeval time)
{
	struct timespec ts;
	bool timed_out;

	ts.tv_sec = time.tv_sec;
	ts.tv_nsec = time.tv_usec * 1000;

    timed_out = pthread_cond_timedwait(&this->cond, &mutex->mutex,
            &ts) == ETIMEDOUT;
	return timed_out;
}

METHOD(cond_t, timed_wait, bool, private_cond_t *this, private_mutex_t *mutex, unsigned int timeout)
{
	struct timeval tv;
	u_int s, ms;

	time_monotonic(&tv);

	s = timeout / 1000;
	ms = timeout % 1000;

	tv.tv_sec += s;
	timeval_add_ms(&tv, ms);
	return timed_wait_abs(this, mutex, tv);
}

METHOD(cond_t, signal_, void, private_cond_t *this)
{
	pthread_cond_signal(&this->cond);
}

METHOD(cond_t, broadcast, void, private_cond_t *this)
{
	pthread_cond_broadcast(&this->cond);
}

METHOD(cond_t, cond_destroy_, void, private_cond_t *this)
{
	pthread_cond_destroy(&this->cond);
	free(this);
}

/*
 * see header file
 */
cond_t *cond_create()
{
    private_cond_t *this;

    INIT(this,
         .public = {
            .wait = (void*)_wait_,
            .timed_wait = (void*)_timed_wait,
            .timed_wait_abs = (void*)_timed_wait_abs,
            .signal = _signal_,
            .broadcast = _broadcast,
            .destroy = _cond_destroy_,
        }
    );

#ifdef HAVE_PTHREAD_CONDATTR_INIT
    pthread_condattr_t condattr;
    pthread_condattr_init(&condattr);
#ifdef HAVE_CONDATTR_CLOCK_MONOTONIC
    pthread_condattr_setclock(&condattr, CLOCK_MONOTONIC);
#endif
    pthread_cond_init(&this->cond, &condattr);
    pthread_condattr_destroy(&condattr);
#endif

    return &this->public;
}

/**
 * @brief destroy cond and free memory
 *
 * @param cond
 */
void cond_destroy(cond_t *cond)
{
    if (cond == NULL) return;
    cond->destroy(cond);
    free(cond);
}
