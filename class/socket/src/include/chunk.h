#ifndef __CHUNK_H__
#define __CHUNK_H__

typedef struct chunk_t chunk_t;
struct chunk_t {
    unsigned char *ptr;
    unsigned int len;
};

extern chunk_t chunk_empty;

/**
 * @brief Create a new chunk pointing to "ptr" with length "len"
 */
inline chunk_t chunk_create(unsigned char *ptr, unsigned int len);

/**
 * @brief Free contents of a chunk
 */
inline void chunk_free(chunk_t *chunk);

/**
 * @brief Overwrite the contents of a chunk and free it
 */
inline void chunk_clear(chunk_t *chunk);

/**
 * @brief Skip n bytes in chunk (forward pointer, shorten length) 
 */
inline chunk_t chunk_skip(chunk_t chunk, unsigned int bytes);

/**
 * @brief Create a clone of a chunk pointing to ptr
 */
inline chunk_t chunk_clone(unsigned char *ptr, chunk_t chunk);

/**
 * @brief Calculate length of multiple chunks
 * @para mode  [in] be 'c' for copy (allocate new chunk), 'm' for move or 's' for sensitive-move (clear given chunk, then free)
 */
int chunk_length(const char *mode, ...);

/**
 * @brief Concatenate chunks into a chunk pointing to "ptr".
 */
chunk_t chunk_create_cat(unsigned char *prt, const char *mode, ...);

/**
 * @brief Split up a chunk into parts, "mode" is a string of "a" (alloc), "c" (copy) and "m" (move).
 *
 * Each letter say for the corresponding chunk if it should get allocated on heap, copied into existing chunk, or the chunk should point into "chunk". The length of each part is an argument before each target chunk.
 *
 * E.g.:
 * chunk_split(chunk, "mcac", 3, &a, 7, &b, 5, &c, d.len, &d);
 */
void chunk_split(chunk_t chunk, const char *mode, ...);

/**
 * @brief Compare two chunks, returns zero if a equals b or negative/positive if a is small/greater than b
 */
int chunk_compare(chunk_t a, chunk_t b);
#endif /* __CHUNK_H__ */
