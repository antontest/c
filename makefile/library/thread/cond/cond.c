#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <utils/utils.h>
#include <cond.h>

/**
 * Add the given number of milliseconds to the given timeval struct
 *
 * @param tv		timeval struct to modify
 * @param ms		number of milliseconds
 */
static void timeval_add_ms_static(struct timeval *tv, u_int ms)
{
    tv->tv_usec += ms * 1000;
    while (tv->tv_usec >= 1000000 /* 1s */)
    {
        tv->tv_usec -= 1000000;
        tv->tv_sec++;
    }
}

/**
 * Get a timestamp from a monotonic time source.
 *
 * While the time()/gettimeofday() functions are affected by leap seconds
 * and system time changes, this function returns ever increasing monotonic
 * time stamps.
 *
 * @param tv		timeval struct receiving monotonic timestamps, or NULL
 * @return			monotonic timestamp in seconds
 */
static long time_monotonic_static(struct timeval *tv)
{
#if defined(HAVE_CLOCK_GETTIME) && \
    (defined(HAVE_CONDATTR_CLOCK_MONOTONIC) || \
     defined(HAVE_PTHREAD_COND_TIMEDWAIT_MONOTONIC))
    /* as we use time_monotonic_static() for condvar operations, we use the
     * monotonic time source only if it is also supported by pthread. */
    timespec_t ts;

    if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0)
    {
        if (tv)
        {
            tv->tv_sec = ts.tv_sec;
            tv->tv_usec = ts.tv_nsec / 1000;
        }
        return ts.tv_sec;
    }
#endif /* HAVE_CLOCK_GETTIME && (...) */
    /* Fallback to non-monotonic timestamps:
     * On MAC OS X, creating monotonic timestamps is rather difficult. We
     * could use mach_absolute_time() and catch sleep/wakeup notifications.
     * We stick to the simpler (non-monotonic) gettimeofday() for now.
     * But keep in mind: we need the same time source here as in condvar! */
    if (!tv)
    {
        return time(NULL);
    }
    if (gettimeofday(tv, NULL) != 0)
    {	/* should actually never fail if passed pointers are valid */
        return -1;
    }
    return tv->tv_sec;
}

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
	unsigned int s, ms;

	time_monotonic_static(&tv);

	s = timeout / 1000;
	ms = timeout % 1000;

	tv.tv_sec += s;
	timeval_add_ms_static(&tv, ms);
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
