#ifndef __UTILS_H__
#define __UTILS_H__
#ifndef _WIN32
#include <sys/time.h>
#endif

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

#ifndef _WIN32 
typedef int SOCKET;
typedef struct sockaddr SOCKADDR;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr_in6 SOCKADDR_IN6;
typedef struct sockaddr_storage SOCKADDR_STORAGE;
#else
#endif

typedef char s8 ;
typedef unsigned char u8 ;
typedef short int s16 ;
typedef unsigned short int u16 ;
typedef int s32 ;
typedef unsigned int u32 ;
typedef enum status_t status_t;

#ifndef _WIN32
#define SNPRINTF(buf, size, fmt, ...) \
    snprintf(buf, size, fmt, ##__VA_ARGS__)
#else
#define SNPRINTF(buf, size, fmt, ...) \
    _snprintf(buf, size, fmt, ##__VA_ARGS__)
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

