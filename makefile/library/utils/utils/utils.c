#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <arpa/inet.h>
#include <utils.h>

/**
 * Helper function that compares two strings for equality
 */
 bool streq(const char *x, const char *y)
{
    return strcmp(x, y) == 0;
}

/**
 * Helper function that compares two strings for equality, length limited
 */
 bool strneq(const char *x, const char *y, unsigned int len)
{
    return strncmp(x, y, len) == 0;
}

/**
 * Helper function that checks if a string starts with a given prefix
 */
 bool strpfx(const char *x, const char *prefix)
{
    return strneq(x, prefix, strlen(prefix));
}

/**
 * Helper function that compares two strings for equality ignoring case
 */
 bool strcaseeq(const char *x, const char *y)
{
    return strcasecmp(x, y) == 0;
}

/**
 * Helper function that compares two strings for equality ignoring case, length limited
 */
 bool strncaseeq(const char *x, const char *y, unsigned int len)
{
    return strncasecmp(x, y, len) == 0;
}

/**
 * NULL-safe strdup variant
 */
 char *strdupnull(const char *s)
{
    return s ? strdup(s) : NULL;
}

/**
 * Helper function that compares two binary blobs for equality
 */
 bool memeq(const void *x, const void *y, unsigned int len)
{
    return memcmp(x, y, len) == 0;
}

/**
 * Add the given number of milliseconds to the given timeval struct
 *
 * @param tv		timeval struct to modify
 * @param ms		number of milliseconds
 */
 void timeval_add_ms(struct timeval *tv, u_int ms)
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
 long time_monotonic(struct timeval *tv)
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
 * Write a 16-bit host order value in network order to an unaligned address.
 *
 * @param host		host order 16-bit value
 * @param network	unaligned address to write network order value to
 */
 void htoun16(void *network, unsigned short host)
{
    char *unaligned = (char*)network;

    host = htons(host);
    memcpy(unaligned, &host, sizeof(host));
}

/**
 * Write a 32-bit host order value in network order to an unaligned address.
 *
 * @param host		host order 32-bit value
 * @param network	unaligned address to write network order value to
 */
 void htoun32(void *network, unsigned int host)
{
    char *unaligned = (char*)network;

    host = htonl(host);
    memcpy((char*)unaligned, &host, sizeof(host));
}

/**
 * Write a 64-bit host order value in network order to an unaligned address.
 *
 * @param host		host order 64-bit value
 * @param network	unaligned address to write network order value to
 */
 void htoun64(void *network, unsigned long int host)
{
    char *unaligned = (char*)network;

#ifdef be64toh
    host = htobe64(host);
    memcpy((char*)unaligned, &host, sizeof(host));
#else
    unsigned int high_part, low_part;

    high_part = host >> 32;
    high_part = htonl(high_part);
    low_part  = host & 0xFFFFFFFFLL;
    low_part  = htonl(low_part);

    memcpy(unaligned, &high_part, sizeof(high_part));
    unaligned += sizeof(high_part);
    memcpy(unaligned, &low_part, sizeof(low_part));
#endif
}

/**
 * Read a 16-bit value in network order from an unaligned address to host order.
 *
 * @param network	unaligned address to read network order value from
 * @return			host order value
 */
 unsigned short untoh16(void *network)
{
    char *unaligned = (char*)network;
    unsigned short tmp;

    memcpy(&tmp, unaligned, sizeof(tmp));
    return ntohs(tmp);
}

/**
 * Read a 32-bit value in network order from an unaligned address to host order.
 *
 * @param network	unaligned address to read network order value from
 * @return			host order value
 */
 unsigned int untoh32(void *network)
{
    char *unaligned = (char*)network;
    unsigned int tmp;

    memcpy(&tmp, unaligned, sizeof(tmp));
    return ntohl(tmp);
}

/**
 * Read a 64-bit value in network order from an unaligned address to host order.
 *
 * @param network	unaligned address to read network order value from
 * @return			host order value
 */
 unsigned long int untoh64(void *network)
{
    char *unaligned = (char*)network;

#ifdef be64toh
    unsigned long int tmp;

    memcpy(&tmp, unaligned, sizeof(tmp));
    return be64toh(tmp);
#else
    unsigned int high_part, low_part;

    memcpy(&high_part, unaligned, sizeof(high_part));
    unaligned += sizeof(high_part);
    memcpy(&low_part, unaligned, sizeof(low_part));

    high_part = ntohl(high_part);
    low_part  = ntohl(low_part);

    return (((unsigned long int)high_part) << 32) + low_part;
#endif
}

/**
 * Round up size to be multiple of alignement
 */
 unsigned int round_up(size_t size, int alignement)
{
    int remainder;

    remainder = size % alignement;
    if (remainder)
    {
        size += alignement - remainder;
    }
    return size;
}

/**
 * Round down size to be a multiple of alignement
 */
 unsigned int round_down(size_t size, int alignement)
{
    return size - (size % alignement);
}

/**
 * @brief rand in array
 *
 * @param min [in] mini number after rand 
 * @param max [in] max number after rand 
 *
 * @return 
 */
 int rand_num(int min, int max)
{
    if (min <0 || min >= max) return -1;
    srandom ((unsigned int)time(NULL));
    return random () % (max - min) + min;
}

/**
 * Safely overwrite n bytes of memory at ptr with zero, auto-inlining variant.
 */
 void memwipe(void *ptr, unsigned int n)
{
    if (!ptr)
    {
        return;
    }
    if (__builtin_constant_p(n))
    {
        memwipe_(ptr, n);
    }
    else
    {
        memwipe_no(ptr, n);
    }
}

/**
 * Safely overwrite n bytes of memory at ptr with zero, inlining variant.
 */
 void memwipe_(void *ptr, unsigned int n)
{
    volatile char *c = (volatile char*)ptr;
    unsigned int m, i;

    /* byte wise until long aligned */
    for (i = 0; (uintptr_t)&c[i] % sizeof(long) && i < n; i++)
    {
        c[i] = 0;
    }
    /* word wise */
    if (n >= sizeof(long))
    {
        for (m = n - sizeof(long); i <= m; i += sizeof(long))
        {
            *(volatile long*)&c[i] = 0;
        }
    }
    /* byte wise of the rest */
    for (; i < n; i++)
    {
        c[i] = 0;
    }
}

/**
 * Described in header.
 */
void memxor(unsigned char dst[], unsigned char  src[], unsigned int n)
{
    int m, i;

    /* byte wise XOR until dst aligned */
    for (i = 0; (uintptr_t)&dst[i] % sizeof(long) && i < n; i++)
    {
        dst[i] ^= src[i];
    }
    /* try to use words if src shares an aligment with dst */
    switch (((uintptr_t)&src[i] % sizeof(long)))
    {
        case 0:
            for (m = n - sizeof(long); i <= m; i += sizeof(long))
            {
                *(long*)&dst[i] ^= *(long*)&src[i];
            }
            break;
        case sizeof(int):
            for (m = n - sizeof(int); i <= m; i += sizeof(int))
            {
                *(int*)&dst[i] ^= *(int*)&src[i];
            }
            break;
        case sizeof(short):
            for (m = n - sizeof(short); i <= m; i += sizeof(short))
            {
                *(short*)&dst[i] ^= *(short*)&src[i];
            }
            break;
        default:
            break;
    }
    /* byte wise XOR of the rest */
    for (; i < n; i++)
    {
        dst[i] ^= src[i];
    }
}

/**
 * Described in header.
 */
void memwipe_no(void *ptr, unsigned int n)
{
	memwipe_(ptr, n);
}

char *ip_netmask(char *ip, int netmask)
{
    unsigned long u_ip = 0;
    int len = sizeof(u_ip) * 8;
    struct in_addr addr;

    if (!ip || netmask < 0 || netmask > 32) 
        return NULL;

    u_ip = inet_addr(ip);
    u_ip = ntohl(u_ip);
    while (++netmask <= len) {
        u_ip &= ~(1 << (len - netmask));
    }
    u_ip = htonl(u_ip);
    memcpy(&addr, &u_ip, sizeof(addr));
    return inet_ntoa(addr);
}
