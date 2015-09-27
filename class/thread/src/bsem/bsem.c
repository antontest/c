#include "utils/utils.h"
#include "mutex.h"
#include "cond.h"
#include "bsem.h"
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

typedef struct private_bsem_t private_bsem_t;

/**
 * private data of a bsem
 */
struct private_bsem_t {
	/**
	 * public interface
	 */
	bsem_t public;

#ifdef HAVE_SEM_TIMEDWAIT
	/**
	 * wrapped POSIX bsem object
	 */
	sem_t sem;
#else /* !HAVE_SEM_TIMEDWAIT */

	/**
	 * Mutex to lock count variable
	 */
	mutex_t *mutex;

	/**
	 * Condvar to signal count increase
	 */
	cond_t *cond;

	/**
	 * bsem count value
	 */
	unsigned int count;
#endif /* HAVE_SEM_TIMEDWAIT */
};

METHOD(bsem_t, wait_, void, private_bsem_t *this)
{
#ifdef HAVE_SEM_TIMEDWAIT
	sem_wait(&this->sem);
#else /* !HAVE_SEM_TIMEDWAIT */
	this->mutex->lock(this->mutex);
	while (this->count == 0)
	{
		this->cond->wait(this->cond, this->mutex);
	}
	this->count--;
	this->mutex->unlock(this->mutex);
#endif /* HAVE_SEM_TIMEDWAIT */
}

METHOD(bsem_t, timed_wait_abs, bool, private_bsem_t *this, struct timeval tv)
{
#ifdef HAVE_SEM_TIMEDWAIT
	timespec_t ts;

	ts.tv_sec = tv.tv_sec;
	ts.tv_nsec = tv.tv_usec * 1000;

	/* there are errors other than ETIMEDOUT possible, but we consider them
	 * all as timeout */
	return sem_timedwait(&this->sem, &ts) == -1;
#else /* !HAVE_SEM_TIMEDWAIT */
	this->mutex->lock(this->mutex);
	while (this->count == 0)
	{
		if (this->cond->timed_wait_abs(this->cond, this->mutex, tv))
		{
			this->mutex->unlock(this->mutex);
			return TRUE;
		}
	}
	this->count--;
	this->mutex->unlock(this->mutex);
	return FALSE;
#endif /* HAVE_SEM_TIMEDWAIT */
}

METHOD(bsem_t, timed_wait, bool, private_bsem_t *this, unsigned int timeout)
{
	struct timeval tv, add;

	add.tv_sec = timeout / 1000;
	add.tv_usec = (timeout % 1000) * 1000;

	time_monotonic(&tv);
	timeradd(&tv, &add, &tv);

	return timed_wait_abs(this, tv);
}

METHOD(bsem_t, post, void, private_bsem_t *this)
{
#ifdef HAVE_SEM_TIMEDWAIT
	sem_post(&this->sem);
#else /* !HAVE_SEM_TIMEDWAIT */
	this->mutex->lock(this->mutex);
	this->count++;
	this->mutex->unlock(this->mutex);
	this->cond->signal(this->cond);
#endif /* HAVE_SEM_TIMEDWAIT */
}

METHOD(bsem_t, destroy, void, private_bsem_t *this)
{
#ifdef HAVE_SEM_TIMEDWAIT
	sem_destroy(&this->sem);
#else /* !HAVE_SEM_TIMEDWAIT */
	this->cond->destroy(this->cond);
	this->mutex->destroy(this->mutex);
#endif /* HAVE_SEM_TIMEDWAIT */
	free(this);
}

/*
 * Described in header
 */
bsem_t *bsem_create(unsigned int value)
{
	private_bsem_t *this;

	INIT(this,
		.public = {
			.wait = _wait_,
			.timed_wait = _timed_wait,
			.timed_wait_abs = _timed_wait_abs,
			.post = _post,
			.destroy = _destroy,
		},
	);

#ifdef HAVE_SEM_TIMEDWAIT
	sem_init(&this->sem, 0, value);
#else /* !HAVE_SEM_TIMEDWAIT */
	this->mutex = mutex_create();
	this->cond = cond_create();
	this->count = value;
#endif /* HAVE_SEM_TIMEDWAIT */

	return &this->public;
}

/**
 * @brief destroy binary semaphore and free memory
 *
 * @param cond
 */
void bsem_destroy(bsem_t *bsem)
{
    if (bsem == NULL) return;
    bsem->destroy(bsem);
    free(bsem);
}
