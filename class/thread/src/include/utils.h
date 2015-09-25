#ifndef __UTILS_H__
#define __UTILS_H__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
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

/**
 * Helper function that compares two strings for equality
 */
static inline bool streq(const char *x, const char *y)
{
	return strcmp(x, y) == 0;
}

/**
 * Helper function that compares two strings for equality, length limited
 */
static inline bool strneq(const char *x, const char *y, size_t len)
{
	return strncmp(x, y, len) == 0;
}

/**
 * Helper function that checks if a string starts with a given prefix
 */
static inline bool strpfx(const char *x, const char *prefix)
{
	return strneq(x, prefix, strlen(prefix));
}

/**
 * Helper function that compares two strings for equality ignoring case
 */
static inline bool strcaseeq(const char *x, const char *y)
{
	return strcasecmp(x, y) == 0;
}

/**
 * Helper function that compares two strings for equality ignoring case, length limited
 */
static inline bool strncaseeq(const char *x, const char *y, size_t len)
{
	return strncasecmp(x, y, len) == 0;
}

/**
 * NULL-safe strdup variant
 */
static inline char *strdupnull(const char *s)
{
	return s ? strdup(s) : NULL;
}

/**
 * Helper function that compares two binary blobs for equality
 */
static inline bool memeq(const void *x, const void *y, size_t len)
{
	return memcmp(x, y, len) == 0;
}

/**
 * Add the given number of milliseconds to the given timeval struct
 *
 * @param tv		timeval struct to modify
 * @param ms		number of milliseconds
 */
static inline void timeval_add_ms(struct timeval *tv, u_int ms)
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
static inline time_t time_monotonic(struct timeval *tv)
{
#if defined(HAVE_CLOCK_GETTIME) && \
	(defined(HAVE_CONDATTR_CLOCK_MONOTONIC) || \
	 defined(HAVE_PTHREAD_COND_TIMEDWAIT_MONOTONIC))
	/* as we use time_monotonic() for condvar operations, we use the
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

#endif

