#include <pthread.h>
#include <signal.h>
#include <semaphore.h>
#include <stdio.h>

#ifdef HAVE_GETTID
#include <sys/types.h>
#include <unistd.h>
#endif

static int threads_init_flags = 0;
static int threads_deinit_flags = 0;
#ifdef HAVE_SYS_GETTID
#include <sys/syscall.h>
static inline pid_t gettid()
{
	return syscall(SYS_gettid);
}
#endif

#include <utils.h>
#include <linked_list/linked_list.h>
#include <mutex/mutex.h>
#include <thread_value/thread_value.h>
#include "thread.h"

typedef struct private_thread_t private_thread_t;

struct private_thread_t {
	/**
	 * Public interface.
	 */
	thread_t public;

	/**
	 * Human-readable ID of this thread.
	 */
	u_int id;

	/**
	 * ID of the underlying thread.
	 */
	pthread_t thread_id;

	/**
	 * Main function of this thread (NULL for the main thread).
	 */
	thread_main_t main;

	/**
	 * Argument for the main function.
	 */
	void *arg;

	/**
	 * Stack of cleanup handlers.
	 */
	linked_list_t *cleanup_handlers;

	/**
	 * Mutex to make modifying thread properties safe.
	 */
	mutex_t *mutex;

	/**
	 * Semaphore used to sync the creation/start of the thread.
	 */
	sem_t created;

	/**
	 * TRUE if this thread has been detached or joined, i.e. can be cleaned
	 * up after terminating.
	 */
	bool detached_or_joined;

	/**
	 * TRUE if the threads has terminated (cancelled, via thread_exit or
	 * returned from the main function)
	 */
	bool terminated;

};

typedef struct {
	/**
	 * Cleanup callback function.
	 */
	thread_cleanup_t cleanup;

	/**
	 * Argument provided to the cleanup function.
	 */
	void *arg;

} cleanup_handler_t;


/**
 * Next thread ID.
 */
static u_int next_id;

/**
 * Mutex to safely access the next thread ID.
 */
static mutex_t *id_mutex;

/**
 * Store the thread object in a thread-specific value.
 */
static thread_value_t *current_thread;


#define HAVE_PTHREAD_CANCEL
#ifndef HAVE_PTHREAD_CANCEL
/* if pthread_cancel is not available, we emulate it using a signal */
#ifdef ANDROID
#define SIG_CANCEL SIGUSR2
#else
#define SIG_CANCEL (SIGRTMIN+7)
#endif

/* the signal handler for SIG_CANCEL uses pthread_exit to terminate the
 * "cancelled" thread */
void cancel_signal_handler(int sig)
{
	pthread_exit(NULL);
}
#endif


/**
 * Destroy an internal thread object.
 *
 * @note The mutex of this thread object has to be locked, it gets unlocked
 * automatically.
 */
static void thread_destroy(private_thread_t *this)
{
	if (!this->terminated || !this->detached_or_joined)
	{
		this->mutex->unlock(this->mutex);
		return;
	}
	this->cleanup_handlers->destroy(this->cleanup_handlers);
	this->mutex->unlock(this->mutex);
	this->mutex->destroy(this->mutex);
	sem_destroy(&this->created);
	free(this);
}

METHOD(thread_t, cancel, void,
	private_thread_t *this)
{
	this->mutex->lock(this->mutex);
	if (pthread_equal(this->thread_id, pthread_self()))
	{
		this->mutex->unlock(this->mutex);
		fprintf(stdout, "!!! CANNOT CANCEL CURRENT THREAD !!!");
		return;
	}
#ifdef HAVE_PTHREAD_CANCEL
	pthread_cancel(this->thread_id);
#else
	pthread_kill(this->thread_id, SIG_CANCEL);
#endif /* HAVE_PTHREAD_CANCEL */
	this->mutex->unlock(this->mutex);
}

METHOD(thread_t, kill_, void,
	private_thread_t *this, int sig)
{
	this->mutex->lock(this->mutex);
	if (pthread_equal(this->thread_id, pthread_self()))
	{
		/* it might actually be possible to send a signal to pthread_self (there
		 * is an example in raise(3) describing that), the problem is though,
		 * that the thread only returns here after the signal handler has
		 * returned, so depending on the signal, the lock might not get
		 * unlocked. */
		this->mutex->unlock(this->mutex);
		fprintf(stdout, "!!! CANNOT SEND SIGNAL TO CURRENT THREAD !!!");
		return;
	}
	pthread_kill(this->thread_id, sig);
	this->mutex->unlock(this->mutex);
}

METHOD(thread_t, detach, void,
	private_thread_t *this)
{
	this->mutex->lock(this->mutex);
	pthread_detach(this->thread_id);
	this->detached_or_joined = TRUE;
	thread_destroy(this);
}

METHOD(thread_t, join, void*,
	private_thread_t *this)
{
	pthread_t thread_id;
	void *val;

	this->mutex->lock(this->mutex);
	if (pthread_equal(this->thread_id, pthread_self()))
	{
		this->mutex->unlock(this->mutex);
		fprintf(stdout, "!!! CANNOT JOIN CURRENT THREAD !!!");
		return NULL;
	}
	if (this->detached_or_joined)
	{
		this->mutex->unlock(this->mutex);
		fprintf(stdout, "!!! CANNOT JOIN DETACHED THREAD !!!");
		return NULL;
	}
	thread_id = this->thread_id;
	this->detached_or_joined = TRUE;
	if (this->terminated)
	{
		/* thread has terminated before the call to join */
		thread_destroy(this);
    }
	else
	{
		/* thread_destroy is called when the thread terminates normally */
		this->mutex->unlock(this->mutex);
	}
	pthread_join(thread_id, &val);

	return val;
}

/**
 * Main cleanup function for threads.
 */
static void thread_cleanup(private_thread_t *this)
{
	cleanup_handler_t *handler;
	this->mutex->lock(this->mutex);
	while (this->cleanup_handlers->remove_last(this->cleanup_handlers,
											   (void**)&handler) == SUCCESS)
	{
        handler->cleanup(handler->arg);
		free(handler);
	}
	this->terminated = TRUE;
	thread_destroy(this);
	this = NULL;
}

METHOD(thread_t, destroy_, void, private_thread_t *this)
{
    thread_cleanup(this);
}

METHOD(thread_t, get_id_, int, private_thread_t *this)
{
    return this->id;
}

/**
 * Create an internal thread object.
 */
static private_thread_t *thread_create_internal()
{
	private_thread_t *this;

	INIT(this,
		.public = {
			.cancel  = _cancel,
			.kill    = _kill_,
			.detach  = _detach,
			.join    = _join,
			.destroy = _destroy_,

			.get_id  = _get_id_,
		},
		.cleanup_handlers = linked_list_create(),
		.mutex = mutex_create(),
	);
	sem_init(&this->created, FALSE, 0);

	return this;
}

/**
 * Main function wrapper for threads.
 */
static void *thread_main(private_thread_t *this)
{
	void *res;

	sem_wait(&this->created);
	current_thread->set(current_thread, this);
	pthread_cleanup_push((thread_cleanup_t)thread_cleanup, this);

	/* TODO: this is not 100% portable as pthread_t is an opaque type (i.e.
	 * could be of any size, or even a struct) */
	/*
#ifdef HAVE_GETTID
	fprintf(stdout, "created thread %.2d [%u]",
		 this->id, gettid());
#else
	fprintf(stdout, "created thread %.2d [%lx]",
		 this->id, (u_long)this->thread_id);
#endif
    */

	res = this->main(this->arg);
	pthread_cleanup_pop(TRUE);
    this = NULL;
    return res;
}

/**
 * Described in header.
 */
thread_t *thread_create(thread_main_t main, void *arg)
{
	private_thread_t *this = thread_create_internal();

	this->main = main;
	this->arg = arg;
	if (pthread_create(&this->thread_id, NULL, (void*)thread_main, this) != 0)
	{
		fprintf(stdout, "failed to create thread!");
		this->mutex->lock(this->mutex);
		thread_destroy(this);
		return NULL;
	}
	id_mutex->lock(id_mutex);
	this->id = next_id++;
	id_mutex->unlock(id_mutex);
	sem_post(&this->created);

	return &this->public;
}

/**
 * Described in header.
 */
thread_t *thread_current()
{
	private_thread_t *this;

	this = (private_thread_t*)current_thread->get(current_thread);
	if (!this)
	{
		this = thread_create_internal();

		id_mutex->lock(id_mutex);
		this->id = next_id++;
		id_mutex->unlock(id_mutex);

		current_thread->set(current_thread, (void*)this);
	}
	return &this->public;
}

/**
 * Described in header.
 */
u_int thread_current_id()
{
	private_thread_t *this = (private_thread_t*)thread_current();

	return this ? this->id : 0;
}

/**
 * Described in header.
 */
void thread_cleanup_push(thread_cleanup_t cleanup, void *arg)
{
	private_thread_t *this = (private_thread_t*)thread_current();
	cleanup_handler_t *handler;

	INIT(handler,
		.cleanup = cleanup,
		.arg = arg,
	);

	this->mutex->lock(this->mutex);
	this->cleanup_handlers->insert_last(this->cleanup_handlers, handler);
	this->mutex->unlock(this->mutex);
}

/**
 * Described in header.
 */
void thread_cleanup_pop(bool execute)
{
	private_thread_t *this = (private_thread_t*)thread_current();
	cleanup_handler_t *handler;

	this->mutex->lock(this->mutex);
	if (this->cleanup_handlers->remove_last(this->cleanup_handlers,
											(void**)&handler) != SUCCESS)
	{
		this->mutex->unlock(this->mutex);
		fprintf(stdout, "!!! THREAD CLEANUP ERROR !!!");
		return;
	}
	this->mutex->unlock(this->mutex);

	if (execute)
	{
		handler->cleanup(handler->arg);
	}
	free(handler);
}

/**
 * Described in header.
 */
bool thread_cancelability(bool enable)
{
#ifdef HAVE_PTHREAD_CANCEL
	int old;

	pthread_setcancelstate(enable ? PTHREAD_CANCEL_ENABLE
								  : PTHREAD_CANCEL_DISABLE, &old);

	return old == PTHREAD_CANCEL_ENABLE;
#else
	sigset_t new, old;

	sigemptyset(&new);
	sigaddset(&new, SIG_CANCEL);
	pthread_sigmask(enable ? SIG_UNBLOCK : SIG_BLOCK, &new, &old);

	return sigismember(&old, SIG_CANCEL) == 0;
#endif /* HAVE_PTHREAD_CANCEL */
}

/**
 * Described in header.
 */
void thread_cancellation_point()
{
	bool old = thread_cancelability(TRUE);

#ifdef HAVE_PTHREAD_CANCEL
	pthread_testcancel();
#endif /* HAVE_PTHREAD_CANCEL */
	thread_cancelability(old);
}

/**
 * Described in header.
 */
void thread_exit(void *val)
{
	pthread_exit(val);
}

/**
 * Described in header.
 */
void threads_init()
{
    if (threads_init_flags) return;
	private_thread_t *main_thread = thread_create_internal();

	main_thread->id = 0;
	main_thread->thread_id = pthread_self();
	current_thread = thread_value_create(NULL);
	current_thread->set(current_thread, (void*)main_thread);
	id_mutex = mutex_create();

#ifndef HAVE_PTHREAD_CANCEL
	{	/* install a signal handler for our custom SIG_CANCEL */
		struct sigaction action = {
			.sa_handler = cancel_signal_handler
		};
		sigaction(SIG_CANCEL, &action, NULL);
	}
#endif /* HAVE_PTHREAD_CANCEL */
    threads_init_flags = 1;
}

/**
 * Described in header.
 */
void threads_deinit()
{
    if (threads_deinit_flags) return;
	private_thread_t *main_thread = (private_thread_t*)thread_current();

	main_thread->mutex->lock(main_thread->mutex);
	thread_destroy(main_thread);
	current_thread->destroy(current_thread);
	id_mutex->destroy(id_mutex);
	threads_deinit_flags = 1;
}

