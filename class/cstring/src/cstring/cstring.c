#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <ctype.h>
#include <cstring.h>
#include <utils.h>

typedef struct private_cstring private_cstring;
struct private_cstring {
    /**
     * @brief public interface
     */
    cstring public;

    /**
     * @brief size of cstring
     */
    int size;

    /**
     * @brief length of string
     */
    int len;

    /**
     * @brief save c replaced
     */
    char *offset;

    /**
     * @brief lock for write
     */
    int lock;

    /**
     * @brief string data
     */
    char data[0];
};

static inline void LOCK(private_cstring *this) 
{
    while (__sync_lock_test_and_set(&(this->lock),1)) {}
}

static inline void UNLOCK(private_cstring *this) 
{
    __sync_lock_release(&(this->lock));
}

METHOD(cstring, set_, char *, private_cstring *this, const char *fmt, ...)
{
    va_list arg;

    LOCK(this);
    va_start(arg, fmt);
    this->len = vsnprintf(this->data, this->size, fmt, arg);
    va_end(arg);
    UNLOCK(this);

    return this->data;
}

METHOD(cstring, add_, char *, private_cstring *this, const char *fmt, ...)
{    
    va_list arg;
    
    LOCK(this);
    va_start(arg, fmt);
    this->len += vsnprintf(this->data + this->len, this->size - this->len, fmt, arg);
    va_end(arg);
    UNLOCK(this);
    
    return this->data;
}

METHOD(cstring, insert_, char *, private_cstring *this, unsigned int index, const char *fmt, ...)
{
    char buf[128] = {0};
    char *a = NULL, *b = NULL;
    va_list arg;
    int len = 0;
    int len_tmp = 0;
    
    if (index > this->len) index = this->len;

    LOCK(this);
    va_start(arg, fmt);
    len = vsnprintf(buf, sizeof(buf), fmt, arg);
    va_end(arg);
    len_tmp = (this->size - this->len - 1) <= len ? (this->size - this->len - 1) : len;

    b = this->data + this->len - 1;
    a = this->data + this->len -1 + len_tmp;
    *(a + 1) = '\0';

    len = this->len - index;
    while (len-- > 0) {
        *a-- = *b--;
    }

    b = this->data + index;
    a = buf;
    len = len_tmp;
    while (len-- > 0) {
        *b++ = *a++;
    }
    this->len += len_tmp;
    UNLOCK(this);

    return this->data;
}

METHOD(cstring, delete_, char *, private_cstring *this, unsigned int index, unsigned int count)
{
    char *p = NULL, *r = NULL;

    if (!count || this->len <= index) return this->data;

    count = (index + count) > this->len ? (this->len - index) : count;
    p = this->data + index + count;
    r = this->data + index;
    LOCK(this);
    while (*p != '\0') {
        *r++ = *p++;
    }
    *r = '\0';
    this->len -= count;
    UNLOCK(this);

    return this->data;
}

METHOD(cstring, get_, char *, private_cstring *this)
{
    return this->data;
}

METHOD(cstring, left_, char *, private_cstring *this, unsigned int count)
{
    if (count >= this->len) return this->data;

    if (this->offset != NULL) free(this->offset);
    this->offset = malloc(count + 1);
    memcpy(this->offset, this->data, count);
    strcat(this->offset, "\0");

    return this->offset;
}

METHOD(cstring, mid_, char *, private_cstring *this, unsigned int start, unsigned int count)
{
    if (start > this->len || (!start && !count)) return NULL;
    if (!count) return this->data + start;
    
    if (this->offset != NULL) free(this->offset);
    this->offset = malloc(count + 1);
    memcpy(this->offset, this->data + start, count);
    strcat(this->offset, "\0");

    return this->offset;
}

METHOD(cstring, right_, char *, private_cstring *this, unsigned int count)
{
    if (count >= this->len) return this->data;

    if (this->offset != NULL) free(this->offset);
    this->offset = malloc(count + 1);
    memcpy(this->offset, this->data + this->len - count, count);
    strcat(this->offset, "\0");

    return this->offset;
}

METHOD(cstring, get_length_, int, private_cstring *this)
{
    return this->len;
}

METHOD(cstring, resize_, int, private_cstring *this, unsigned int size)
{
    LOCK(this);
    char *p = realloc((void *)this, sizeof(private_cstring) + size);
    if (!p) return -1;
    this = (private_cstring *)p;
    this->size = size;
    UNLOCK(this);

    return 0;
}

METHOD(cstring, destory_, void, private_cstring *this)
{
    if (this->offset != NULL) free(this->offset);
    free(this);
}

METHOD(cstring, get_size, int, private_cstring *this)
{
    return this->size;
}

METHOD(cstring, toint_, int, private_cstring *this)
{
    return strtol(this->data, NULL, 10);
}

METHOD(cstring, tolower_, char *, private_cstring *this)
{
    char *s = this->data;

    LOCK(this);
    while (*s != '\0') {
        if (isupper(*s)) *s = tolower(*s);
        s++;
    }
    UNLOCK(this);
    
    return this->data;    
}

METHOD(cstring, toupper_, char *, private_cstring *this)
{
    char *s = this->data;

    LOCK(this);
    while (*s != '\0') {
        if (islower(*s)) *s = toupper(*s);
        s++;
    }
    UNLOCK(this);
    
    return this->data;    
}

METHOD(cstring, left_trim_, char *, private_cstring *this)
{
    char *p = this->data, *q = this->data;

    LOCK(this);
    while (*p == ' ') p++;
    while ((*q++ = *p++) != '\0') NULL;
    this->len = strlen(this->data);
    UNLOCK(this);

    return this->data;
}

METHOD(cstring, right_trim_, char *, private_cstring *this)
{
    char *p = this->data;

    LOCK(this);
    while (*p != '\0') p++;
    while (--p != this->data && *p == ' ') NULL;
    *(++p) = '\0';
    this->len = strlen(this->data);
    UNLOCK(this);

    return this->data;
}

METHOD(cstring, mid_trim_, char *, private_cstring *this)
{
    char *a = this->data, *b = this->data;

    LOCK(this);
    while (*a != '\0' && *a == ' ') a++;
    *b = *a;

    while (*(++a) != '\0') {
        if (*a != ' ' || *b != ' ') *(++b) = *a;
    }

    if (*b == ' ') *b = '\0';
    else *(++b) = '\0';
    this->len = strlen(this->data);
    UNLOCK(this);

    return this->data;
}

METHOD(cstring, all_trim_, char *, private_cstring *this)
{
    char *s = this->data, *b = this->data;

    LOCK(this);
    while (*s != '\0') {
        if (*s != ' ') *b++ = *s;
        s++;
    }
    *b = '\0';
    this->len = strlen(this->data);
    UNLOCK(this);
    
    return this->data;
}

METHOD(cstring, cmp_, inline int, private_cstring *this, const char *s)
{
    return s != NULL ? strcmp(this->data, s) : -1;
}

METHOD(cstring, casecmp_, inline int, private_cstring *this, const char *s)
{
    return s != NULL ? strcasecmp(this->data, s) : -1;
}

METHOD(cstring, empty_, void, private_cstring *this)
{
    LOCK(this);
    memset(this->data, 0, this->size);
    UNLOCK(this);
}

METHOD(cstring, is_empty_, int, private_cstring *this)
{
    return !this->len ? 1: 0;
}

/**
 * @brief Create cstring instance 
 */
cstring *create_cstring(unsigned int size)
{
    private_cstring *this;

    this = malloc(sizeof(private_cstring) + size);
    if (!this) return NULL;
    (*this) = (private_cstring) {
        .public = {
            .set        = _set_,
            .add        = _add_,
            .insert     = _insert_,
            .delete     = _delete_,
            .resize     = _resize_,
            .destroy    = _destory_,

            .get        = _get_,
            .left       = _left_,
            .mid        = _mid_,
            .right      = _right_,

            .get_length = _get_length_,
            .get_size   = _get_size,

            .toint      = _toint_,
            .tolower    = _tolower_,
            .toupper    = _toupper_,

            .left_trim  = _left_trim_,
            .mid_trim   = _mid_trim_,
            .right_trim = _right_trim_,
            .all_trim   = _all_trim_,

            .cmp        = _cmp_,
            .casecmp    = _casecmp_,

            .empty      = _empty_,
            .is_empty   = _is_empty_,
        },
        .size   = size,
        .len    = 0,
        .offset = NULL,
    };

    return &this->public;
}
