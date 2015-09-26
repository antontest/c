#include <pthread.h>
#include <stdio.h>
#include <utils/utils.h>
#include "cond.h"
#include "thread.h"
#include "mutex.h"
#include "rwlock.h"
#include "rwlock_cond.h"


typedef struct private_rwlock_t private_rwlock_t;
typedef struct private_rwlock_cond_t private_rwlock_cond_t;

/**
 * private data of rwlock
 */
struct private_rwlock_t {

	/**
	 * public functions
	 */
	rwlock_t public;

#ifdef HAVE_PTHREAD_RWLOCK_INIT

	/**
	 * wrapped pthread rwlock
	 */
	pthread_rwlock_t rwlock;

#else

	/**
	 * mutex to emulate a native rwlock
	 */
	mutex_t *mutex;

	/**
	 * cond to handle writers
	 */
	cond_t *writers;

	/**
	 * cond to handle readers
	 */
	cond_t *readers;

	/**
	 * number of waiting writers
	 */
	u_int waiting_writers;

	/**
	 * number of readers holding the lock
	 */
	u_int reader_count;

	/**
	 * TRUE, if a writer is holding the lock currently
	 */
	bool writer;

#endif /* HAVE_PTHREAD_RWLOCK_INIT */

};

/**
 * private data of cond
 */
struct private_rwlock_cond_t {

	/**
	 * public interface
	 */
	rwlock_cond_t public;

	/**
	 * mutex used to implement rwlock cond
	 */
	mutex_t *mutex;

	/**
	 * regular cond to implement rwlock cond
	 */
	cond_t *cond;
};


#ifdef HAVE_PTHREAD_RWLOCK_INIT

METHOD(rwlock_t, read_lock, void,
	private_rwlock_t *this)
{
	int err;

	err = pthread_rwlock_rdlock(&this->rwlock);
	if (err != 0)
	{
		fprintf(stdout, "!!! RWLOCK READ LOCK ERROR: %s !!!", strerror(err));
	}
}

METHOD(rwlock_t, write_lock, void,
	private_rwlock_t *this)
{
	int err;

	err = pthread_rwlock_wrlock(&this->rwlock);
	if (err != 0)
	{
		fprintf(stdout, "!!! RWLOCK WRITE LOCK ERROR: %s !!!", strerror(err));
	}
}

METHOD(rwlock_t, try_write_lock, bool,
	private_rwlock_t *this)
{
	return pthread_rwlock_trywrlock(&this->rwlock) == 0;
}

METHOD(rwlock_t, unlock, void,
	private_rwlock_t *this)
{
	int err;

	err = pthread_rwlock_unlock(&this->rwlock);
	if (err != 0)
	{
		fprintf(stdout, "!!! RWLOCK UNLOCK ERROR: %s !!!", strerror(err));
	}
}

METHOD(rwlock_t, destroy, void,
	private_rwlock_t *this)
{
	pthread_rwlock_destroy(&this->rwlock);
	free(this);
}

/*
 * see header file
 */
rwlock_t *rwlock_create()
{
			private_rwlock_t *this;

			INIT(this,
				.public = {
					.read_lock = _read_lock,
					.write_lock = _write_lock,
					.try_write_lock = _try_write_lock,
					.unlock = _unlock,
					.destroy = _destroy,
				}
			);

			pthread_rwlock_init(&this->rwlock, NULL);

			return &this->public;
}

#else /* HAVE_PTHREAD_RWLOCK_INIT */

/**
 * This implementation of the rwlock_t interface uses mutex_t and cond_t
 * primitives, if the pthread_rwlock_* group of functions is not available or
 * don't allow recursive locking for readers.
 *
 * The following constraints are enforced:
 *   - Multiple readers can hold the lock at the same time.
 *   - Only a single writer can hold the lock at any given time.
 *   - A writer must block until all readers have released the lock before
 *     obtaining the lock exclusively.
 *   - Readers that don't hold any read lock and arrive while a writer is
 *     waiting to acquire the lock will block until after the writer has
 *     obtained and released the lock.
 * These constraints allow for read sharing, prevent write sharing, prevent
 * read-write sharing and (largely) prevent starvation of writers by a steady
 * stream of incoming readers.  Reader starvation is not prevented (this could
 * happen if there are more writers than readers).
 *
 * The implementation supports recursive locking of the read lock but not of
 * the write lock.  Readers must not acquire the lock exclusively at the same
 * time and vice-versa (this is not checked or enforced so behave yourself to
 * prevent deadlocks).
 *
 * Since writers are preferred a thread currently holding the read lock that
 * tries to acquire the read lock recursively while a writer is waiting would
 * result in a deadlock.  In order to avoid having to use a thread-specific
 * value for each rwlock_t (or a list of threads) to keep track if a thread
 * already acquired the read lock we use a single thread-specific value for all
 * rwlock_t objects that keeps track of how many read locks a thread currently
 * holds.  Preferring readers that already hold ANY read locks prevents this
 * deadlock while it still largely avoids writer starvation (for locks that can
 * only be acquired while holding another read lock this will obviously not
 * work).
 */

/**
 * Keep track of how many read locks a thread holds.
 */
static pthread_key_t is_reader;

/**
 * Only initialize the read lock counter once.
 */
static pthread_once_t is_reader_initialized = PTHREAD_ONCE_INIT;

/**
 * Initialize the read lock counter.
 */
static void initialize_is_reader()
{
	pthread_key_create(&is_reader, NULL);
}

METHOD(rwlock_t, read_lock, void,
	private_rwlock_t *this)
{
	uintptr_t reading;

	reading = (uintptr_t)pthread_getspecific(is_reader);
	this->mutex->lock(this->mutex);
	if (!this->writer && reading > 0)
	{
		/* directly allow threads that hold ANY read locks, to avoid a deadlock
		 * caused by preferring writers in the loop below */
	}
	else
	{
		while (this->writer || this->waiting_writers)
		{
			this->readers->wait(this->readers, this->mutex);
		}
	}
	this->reader_count++;
	this->mutex->unlock(this->mutex);
	pthread_setspecific(is_reader, (void*)(reading + 1));
}

METHOD(rwlock_t, write_lock, void,
	private_rwlock_t *this)
{
	this->mutex->lock(this->mutex);
	this->waiting_writers++;
	while (this->writer || this->reader_count)
	{
		this->writers->wait(this->writers, this->mutex);
	}
	this->waiting_writers--;
	this->writer = TRUE;
	this->mutex->unlock(this->mutex);
}

METHOD(rwlock_t, try_write_lock, bool,
	private_rwlock_t *this)
{
	bool res = FALSE;
	this->mutex->lock(this->mutex);
	if (!this->writer && !this->reader_count)
	{
		res = this->writer = TRUE;
	}
	this->mutex->unlock(this->mutex);
	return res;
}

METHOD(rwlock_t, unlock, void,
	private_rwlock_t *this)
{
	this->mutex->lock(this->mutex);
	if (this->writer)
	{
		this->writer = FALSE;
	}
	else
	{
		uintptr_t reading;

		this->reader_count--;
		reading = (uintptr_t)pthread_getspecific(is_reader);
		pthread_setspecific(is_reader, (void*)(reading - 1));
	}
	if (!this->reader_count)
	{
		if (this->waiting_writers)
		{
			this->writers->signal(this->writers);
		}
		else
		{
			this->readers->broadcast(this->readers);
		}
	}
	this->mutex->unlock(this->mutex);
}

METHOD(rwlock_t, destroy, void,
	private_rwlock_t *this)
{
	this->mutex->destroy(this->mutex);
	this->writers->destroy(this->writers);
	this->readers->destroy(this->readers);
	free(this);
}

/*
 * see header file
 */
rwlock_t *rwlock_create()
{
	pthread_once(&is_reader_initialized,  initialize_is_reader);

    private_rwlock_t *this;

    INIT(this,
            .public = {
            .read_lock = _read_lock,
            .write_lock = _write_lock,
            .try_write_lock = _try_write_lock,
            .unlock = _unlock,
            .destroy = _destroy,
            },
            .mutex = mutex_create(),
            .writers = cond_create(),
            .readers = cond_create(),
        );

    return &this->public;
}

METHOD(rwlock_cond_t, wait_, void,
	private_rwlock_cond_t *this, rwlock_t *lock)
{
	/* at this point we have the write lock locked, to make signals more
	 * predictable we try to prevent other threads from signaling by acquiring
	 * the mutex while we still hold the write lock (this assumes they will
	 * hold the write lock themselves when signaling, which is not mandatory) */
	this->mutex->lock(this->mutex);
	/* unlock the rwlock and wait for a signal */
	lock->unlock(lock);
	/* if the calling thread enabled thread cancelability we want to replicate
	 * the behavior of the regular cond, i.e. the lock will be held again
	 * before executing cleanup functions registered by the calling thread */
	thread_cleanup_push((thread_cleanup_t)lock->write_lock, lock);
	thread_cleanup_push((thread_cleanup_t)this->mutex->unlock, this->mutex);
	this->cond->wait(this->cond, this->mutex);
	/* we release the mutex to allow other threads into the cond (might even
	 * be required so we can acquire the lock again below) */
	thread_cleanup_pop(TRUE);
	/* finally we reacquire the lock we held previously */
	thread_cleanup_pop(TRUE);
}

METHOD(rwlock_cond_t, timed_wait_abs, bool,
	private_rwlock_cond_t *this, rwlock_t *lock, struct timeval time)
{
	bool timed_out;

	/* see wait() above for details on what is going on here */
	this->mutex->lock(this->mutex);
	lock->unlock(lock);
	thread_cleanup_push((thread_cleanup_t)lock->write_lock, lock);
	thread_cleanup_push((thread_cleanup_t)this->mutex->unlock, this->mutex);
	timed_out = this->cond->timed_wait_abs(this->cond, this->mutex, time);
	thread_cleanup_pop(TRUE);
	thread_cleanup_pop(!timed_out);
	return timed_out;
}

METHOD(rwlock_cond_t, timed_wait, bool,
	private_rwlock_cond_t *this, rwlock_t *lock, u_int timeout)
{
	struct timeval tv;
	u_int s, ms;

	time_monotonic(&tv);

	s = timeout / 1000;
	ms = timeout % 1000;

	tv.tv_sec += s;
	timeval_add_ms(&tv, ms);

	return timed_wait_abs(this, lock, tv);
}

METHOD(rwlock_cond_t, signal_, void,
	private_rwlock_cond_t *this)
{
	this->mutex->lock(this->mutex);
	this->cond->signal(this->cond);
	this->mutex->unlock(this->mutex);
}

METHOD(rwlock_cond_t, broadcast, void,
	private_rwlock_cond_t *this)
{
	this->mutex->lock(this->mutex);
	this->cond->broadcast(this->cond);
	this->mutex->unlock(this->mutex);
}

METHOD(rwlock_cond_t, condvar_destroy, void,
	private_rwlock_cond_t *this)
{
	this->cond->destroy(this->cond);
	this->mutex->destroy(this->mutex);
	free(this);
}

/*
 * see header file
 */
rwlock_cond_t *rwlock_cond_create()
{
	private_rwlock_cond_t *this;

	INIT(this,
		.public = {
			.wait = _wait_,
			.timed_wait = _timed_wait,
			.timed_wait_abs = _timed_wait_abs,
			.signal = _signal_,
			.broadcast = _broadcast,
			.destroy = _condvar_destroy,
		},
		.mutex = mutex_create(),
		.cond = cond_create(),
	);
	return &this->public;
}


#endif /* HAVE_PTHREAD_RWLOCK_INIT */

