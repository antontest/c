#include <chunk.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <utils/utils.h>

chunk_t chunk_empty = {NULL, 0};

inline chunk_t chunk_create(unsigned char *ptr, unsigned int len)
{
    chunk_t chunk = {ptr, len};
    return chunk;
}

inline void chunk_free(chunk_t *chunk)
{
    if (chunk && chunk->ptr) free(chunk->ptr);
    if (chunk) *chunk = chunk_empty;
}

inline void chunk_clear(chunk_t *chunk)
{
    if (chunk && chunk->ptr) {
        memset(chunk->ptr, 0, chunk->len);
        chunk_free(chunk);
    }
}

inline chunk_t chunk_skip(chunk_t chunk, unsigned int bytes)
{
    if (chunk.len > bytes) {
        chunk.ptr += bytes;
        chunk.len -= bytes;
        return chunk;
    }

    return chunk_empty;
}

inline chunk_t chunk_clone(unsigned char *ptr, chunk_t chunk)
{
    chunk_t clone = chunk_empty;
    if (chunk.ptr && chunk.len > 0) {
        clone.ptr = ptr;
        clone.len = chunk.len;
        memcpy(clone.ptr, chunk.ptr, chunk.len);
    }

    return clone;
}

int chunk_length(const char *mode, ...)
{
    va_list chunks;
    int len = 0;
    chunk_t ch;

    va_start(chunks, mode);
    while (1) {
        switch (*mode++) {
            case 'm':
            case 'c':
            case 's':
                ch = va_arg(chunks, chunk_t);
                len += ch.len;
                continue;
            default:
                break;
        }
        break;
    }
    va_end(chunks);

    return len;
}

chunk_t chunk_create_cat(unsigned char *ptr, const char *mode, ...)
{
    int clear_chunk = 0;
    int free_chunk = 0;
    va_list chunks;
    chunk_t construct = chunk_create(ptr, 0);
    chunk_t ch;

    va_start(chunks, mode);
    while (1) {
        switch (*mode++) {
            case 's':
                clear_chunk = 1;
            case 'm':
                free_chunk = 1;
            case 'c':
                ch = va_arg(chunks, chunk_t);
                memcpy(ptr, ch.ptr, ch.len);
                ptr += ch.len;
                construct.len += ch.len;

                if (clear_chunk) chunk_free(&ch);
                else if (free_chunk) free(ch.ptr);
                continue;
        }
        break;
    }
    va_end(chunks);

    return construct;
}

void chunk_split(chunk_t chunk, const char *mode, ...)
{
    unsigned int len;
    va_list chunks;
    chunk_t *ch;

    va_start(chunks, mode);
    while (1) {
        if (*mode == '\0') break;

        len = va_arg(chunks, unsigned int);
        ch = va_arg(chunks, chunk_t *);

        if (!ch) {
            chunk = chunk_skip(chunk, len);
            continue;
        }

        switch (*mode++) {
            case 'm':
                ch->len = min(chunk.len, len);
                if (ch->len) ch->ptr = chunk.ptr;
                else ch->ptr = NULL;
                chunk = chunk_skip(chunk, ch->len);
                continue;
            case 'a':
                ch->len = min(chunk.len, len);
                if (ch->len) {
                    ch->ptr = malloc(ch->len);
                    memcpy(ch->ptr, ch->ptr, ch->len);
                } else ch->ptr = NULL;
                chunk = chunk_skip(chunk, ch->len);
                continue;
            case 'c':
                ch->len = min(chunk.len, len);
                if (ch->len) memcpy(ch->ptr, chunk.ptr, ch->len);
                else ch->ptr = NULL;
                chunk = chunk_skip(chunk, ch->len);
                continue;
            default:
                break;
        }
        break;
    }
    va_end(chunks);
}

int chunk_compare(chunk_t a, chunk_t b)
{
    int compare_len = a.len - b.len;
    int len = compare_len < 0 ? a.len : b.len;

    if (compare_len != 0 || len == 0)
        return compare_len;

    return memcmp(a.ptr, b.ptr, len);
}
