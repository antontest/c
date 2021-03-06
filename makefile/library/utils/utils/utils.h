#ifndef __UTILS_H__
#define __UTILS_H__
#ifndef _WIN32
#include <sys/time.h>
#endif
#include <time.h>

/**
* General purpose boolean type.
*/
#ifdef HAVE_STDBOOL_H
# include <stdbool.h>
#else
# ifndef HAVE__BOOL
#  define _Bool signed char
# endif /* HAVE__BOOL */
# define bool _Bool
# define false 0
# define true 1
# define __bool_true_false_are_defined 1
#endif /* HAVE_STDBOOL_H */
#ifndef FALSE
# define FALSE false
#endif /* FALSE */
#ifndef TRUE
# define TRUE  true
#endif /* TRUE */

typedef char s8 ;
typedef unsigned char u8 ;
typedef short int s16 ;
typedef unsigned short int u16 ;
typedef int s32 ;
typedef unsigned int u32 ;
typedef enum status_t status_t;
/**
 * Return values of function calls.
 */
enum status_t {
	/**
	 * Call succeeded.
	 */
	SUCCESS,

	/**
	 * Call failed.
	 */
	FAILED,

	/**
	 * Out of resources.
	 */
	OUT_OF_RES,

	/**
	 * The suggested operation is already done
	 */
	ALREADY_DONE,

	/**
	 * Not supported.
	 */
	NOT_SUPPORTED,

	/**
	 * One of the arguments is invalid.
	 */
	INVALID_ARG,

	/**
	 * Something could not be found.
	 */
	NOT_FOUND,

	/**
	 * Error while parsing.
	 */
	PARSE_ERROR,

	/**
	 * Error while verifying.
	 */
	VERIFY_ERROR,

	/**
	 * Object in invalid state.
	 */
	INVALID_STATE,

	/**
	 * Destroy object which called method beints to.
	 */
	DESTROY_ME,

	/**
	 * Another call to the method is required.
	 */
	NEED_MORE,
};


#ifndef _WIN32
#define SNPRINTF(buf, size, fmt, ...) \
    snprintf(buf, size, fmt, ##__VA_ARGS__)
#else
#define SNPRINTF(buf, size, fmt, ...) \
    _snprintf(buf, size, fmt, ##__VA_ARGS__)
#endif

#ifndef _WIN32 
#define ACCESS(path, mode) access(path, mode)
#else 
#define ACCESS(path, mode) _access(path, mode)
#endif

/**
 * mutex
 */
#ifndef _WIN32
#include <pthread.h>
typedef pthread_mutex_t    MUTEX_HANDLE;
#define MUTEX_LOCK(mtx)    pthread_mutex_lock(&(mtx))
#define MUTEX_UNLOCK(mtx)  pthread_mutex_unlock(&(mtx))
#define MUTEX_DESTROY(mtx) pthread_mutex_destroy(&(mtx))
#else
typedef void *             MUTEX_HANDLE;
#define MUTEX_LOCK(mtx)    WaitForSingleObject(mtx,INFINITE)
#define MUTEX_UNLOCK(mtx)  ReleaseMutex(mtx)
#define MUTEX_DESTROY(mtx) CloseHandle(mtx)
#endif /* _WIN32 */

/**
 * sem
 */
#ifndef _WIN32
#include <semaphore.h>
typedef sem_t SEM_HANDLE;
#define SEM_INIT(sem, value) sem_init(&(sem), 0, value)
#define SEM_WAIT(sem) sem_wait(&(sem))
#define SEM_POST(sem) sem_post(&(sem))
#define SEM_DESTROY(sem) sem_destroy(&sem)
#else
typedef void * SEM_HANDLE;
#define SEM_INIT(sem, value) (sem) = CreateSemaphore(NULL, value, value + 1, NULL)
#define SEM_WAIT(sem) WaitForSingleObject(sem, INFINITE)
#define SEM_POST(sem) ReleaseSemaphore(sem, 1, NULL)
#define SEM_DESTROY(sem) CloseHandle(sem)
#endif /* _WIN32 */

/**
 * socket
 */
#ifndef _WIN32 
typedef int SOCKET;
typedef struct sockaddr SOCKADDR;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr_in6 SOCKADDR_IN6;
typedef struct sockaddr_storage SOCKADDR_STORAGE;
#endif

/**
 * time
 */ 
#ifndef _WIN32
typedef struct timeval tclock_t;
#define TIME_PER_SEC 1000000
#define GETCURRTIME(tm) gettimeofday(&(tm), NULL)
#define GETCOSTTIME(end_time, start_time) (double)(((end_time.tv_sec - start_time.tv_sec) * TIME_PER_SEC + end_time.tv_usec - start_time.tv_usec) / ((double)TIME_PER_SEC))
#else 
typedef clock_t tclock_t;
#define TIME_PER_SEC 1000
#define GETCURRTIME(tm) (tm) = clock()
#define GETCOSTTIME(start_time, end_time) cost = (((double)(end_time - start_time)) / ((double)TIME_PER_SEC))
#endif

/**
* Call destructor of an object, if object != NULL
*/
#define DESTROY_IF(obj) if (obj) (obj)->destroy(obj)

/**
* Call offset destructor of an object, if object != NULL
*/
#define DESTROY_OFFSET_IF(obj, offset) if (obj) obj->destroy_offset(obj, offset);

/**
* Call function destructor of an object, if object != NULL
*/
#define DESTROY_FUNCTION_IF(obj, fn) if (obj) obj->destroy_function(obj, fn);


/**
* Macro gives back larger of two values.
*/
#ifndef _WIN32
#define max(x,y) ({ \
	typeof(x) _x = (x); \
	typeof(y) _y = (y); \
	_x > _y ? _x : _y; })


/**
* Macro gives back smaller of two values.
*/
#define min(x,y) ({ \
	typeof(x) _x = (x); \
	typeof(y) _y = (y); \
	_x < _y ? _x : _y; })
#endif

/**
* Debug macro to follow control flow
*/
#define POS printf("%s, line %d\n", __FILE__, __LINE__)

/**
* get variable address
*/
#define ADDR(var) ((void *)&var)

/**
* get address's address
*/
#define ADDR_ADDR(var) ((void **)&var)

/**
* Object allocation/initialization macro, using designated initializer.
*/
#ifndef _WIN32
#define INIT(this, ...) \
{ \
	(this) = malloc(sizeof(*(this))); \
	*(this) = (typeof(*(this))){ __VA_ARGS__ }; \
	}
#else
#define INIT(self, type, ...) \
{ \
	type tmp = { __VA_ARGS__ }; \
	(self) = malloc(sizeof(*(self))); \
	memcpy((self), &tmp, sizeof(tmp)); \
}
#endif

/**
* Method declaration/definition macro, providing private and public interface.
*
* Defines a method name with this as first parameter and a return value ret,
* and an alias for this method with a _ prefix, having the this argument
* safely casted to the public interface iface.
* _name is provided a function pointer, but will get optimized out by GCC.
*/
#ifndef _WIN32
#define METHOD(iface, name, ret, this, ...) \
	static ret name(union {iface *_public; this;} \
	__attribute__((transparent_union)), ##__VA_ARGS__); \
	static typeof(name) *_##name = (typeof(name)*)name; \
	static ret name(this, ##__VA_ARGS__)
#else
#define METHOD(iface, name, ret, self, ...) \
	static ret name(self, ##__VA_ARGS__)
#endif

/**
* Same as METHOD(), but is defined for two public interfaces.
*/
#define METHOD2(iface1, iface2, name, ret, this, ...) \
	static ret name(union {iface1 *_public1; iface2 *_public2; this;} \
	__attribute__((transparent_union)), ##__VA_ARGS__); \
	static typeof(name) *_##name = (typeof(name)*)name; \
	static ret name(this, ##__VA_ARGS__)


/**
* Macro to allocate a sized type.
*/
#define malloc_thing(thing) ((thing*)malloc(sizeof(thing)))

/**
* Get the number of elements in an array
*/
#define countof(array) (sizeof(array)/sizeof(array[0]))

/**
* Ignore result of functions tagged with warn_unused_result attributes
*/
#define ignore_result(call) { if(call){}; }

/**
* Assign a function as a class method
*/
#define ASSIGN(method, function) (method = (typeof(method))function)

#endif

