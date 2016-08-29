#ifndef __UTILS_H__
#define __UTILS_H__
#ifndef _Win32
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
#define INIT(this, ...) \
{ \
    (this) = malloc(sizeof(*(this))); \
	*(this) = (typeof(*(this))){ __VA_ARGS__ }; \
}

/**
 * Method declaration/definition macro, providing private and public interface.
 *
 * Defines a method name with this as first parameter and a return value ret,
 * and an alias for this method with a _ prefix, having the this argument
 * safely casted to the public interface iface.
 * _name is provided a function pointer, but will get optimized out by GCC.
 */
#define METHOD(iface, name, ret, this, ...) \
	static ret name(union {iface *_public; this;} \
	__attribute__((transparent_union)), ##__VA_ARGS__); \
	static typeof(name) *_##name = (typeof(name)*)name; \
	static ret name(this, ##__VA_ARGS__)

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

/**
 * Helper function that compares two strings for equality
 */
inline bool streq(const char *x, const char *y);

/**
 * Helper function that compares two strings for equality, length limited
 */
inline bool strneq(const char *x, const char *y, unsigned int len);

/**
 * Helper function that checks if a string starts with a given prefix
 */
inline bool strpfx(const char *x, const char *prefix);

/**
 * Helper function that compares two strings for equality ignoring case
 */
inline bool strcaseeq(const char *x, const char *y);

/**
 * Helper function that compares two strings for equality ignoring case, length limited
 */
inline bool strncaseeq(const char *x, const char *y, unsigned int len);

/**
 * NULL-safe strdup variant
 */
inline char *strdupnull(const char *s);

/**
 * Helper function that compares two binary blobs for equality
 */
inline bool memeq(const void *x, const void *y, unsigned int len);

/**
 * Add the given number of milliseconds to the given timeval struct
 *
 * @param tv		timeval struct to modify
 * @param ms		number of milliseconds
 */
inline void timeval_add_ms(struct timeval *tv, unsigned int ms);

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
inline long time_monotonic(struct timeval *tv);

/**
 * Write a 16-bit host order value in network order to an unaligned address.
 *
 * @param host		host order 16-bit value
 * @param network	unaligned address to write network order value to
 */
inline void htoun16(void *network, unsigned short host);

/**
 * Write a 32-bit host order value in network order to an unaligned address.
 *
 * @param host		host order 32-bit value
 * @param network	unaligned address to write network order value to
 */
inline void htoun32(void *network, unsigned int host);

/**
 * Write a 64-bit host order value in network order to an unaligned address.
 *
 * @param host		host order 64-bit value
 * @param network	unaligned address to write network order value to
 */
inline void htoun64(void *network, unsigned long int  host);

/**
 * Read a 16-bit value in network order from an unaligned address to host order.
 *
 * @param network	unaligned address to read network order value from
 * @return			host order value
 */
inline unsigned short untoh16(void *network);

/**
 * Read a 32-bit value in network order from an unaligned address to host order.
 *
 * @param network	unaligned address to read network order value from
 * @return			host order value
 */
inline unsigned int untoh32(void *network);

/**
 * Read a 64-bit value in network order from an unaligned address to host order.
 *
 * @param network	unaligned address to read network order value from
 * @return			host order value
 */
inline unsigned long int untoh64(void *network);

/**
 * Round up size to be multiple of alignement
 */
inline unsigned int round_up(unsigned int size, int alignement);

/**
 * Round down size to be a multiple of alignement
 */
inline unsigned int round_down(unsigned int size, int alignement);

/**
 * @brief rand in array
 *
 * @param min [in] mini number after rand 
 * @param max [in] max number after rand 
 *
 * @return a number in min ~ max
 */
inline int rand_num(int min, int max);

/**
 * Same as memcpy, but XORs src into dst instead of copy
 */
void memxor(unsigned char dest[], unsigned char src[], unsigned int n);

/**
 * Safely overwrite n bytes of memory at ptr with zero, inlining variant.
 */
inline void memwipe_inline(void *ptr, unsigned int n);

/**
 * Safely overwrite n bytes of memory at ptr with zero, auto-inlining variant.
 */
inline void memwipe(void *ptr, unsigned int n);

/**
 * Described in header.
 */
void memwipe_noinline(void *ptr, unsigned int n);

/**
 * @brief get netmask ip by netmask
 *
 * @param 172.21.34.21/24 -- 172.21.34.0
 */
char *ip_netmask(char *ip, int netmask);

#endif

