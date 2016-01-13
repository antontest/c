#include <pthread.h>
#include "utils.h"
#include "thread_value.h"

typedef struct private_thread_value_t private_thread_value_t;

struct private_thread_value_t {
	/**
	 * Public interface.
	 */
	thread_value_t public;

	/**
	 * Key to access thread-specific values.
	 */
	pthread_key_t key;

	/**
	 * Destructor to cleanup the value of the thread destroying this object
	 */
	thread_cleanup_t destructor;

};

METHOD(thread_value_t, set, void,
	private_thread_value_t *this, void *val)
{
	pthread_setspecific(this->key, val);
}

METHOD(thread_value_t, get, void*,
	private_thread_value_t *this)
{
	return pthread_getspecific(this->key);
}

METHOD(thread_value_t, destroy, void,
	private_thread_value_t *this)
{
	void *val;

	/* the destructor is not called automatically for the thread calling
	 * pthread_key_delete() */
	if (this->destructor)
	{
		val = pthread_getspecific(this->key);
		if (val)
		{
			this->destructor(val);
		}
	}
	pthread_key_delete(this->key);
	free(this);
}

/**
 * Described in header.
 */
thread_value_t *thread_value_create(thread_cleanup_t destructor)
{
	private_thread_value_t *this;

	INIT(this,
		.public = {
			.set = _set,
			.get = _get,
			.destroy = _destroy,
		},
		.destructor = destructor,
	);

	pthread_key_create(&this->key, destructor);
	return &this->public;
}

