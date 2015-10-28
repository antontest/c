
/**
 * @defgroup chunk chunk
 * @{ @ingroup utils
 */

#ifndef CHUNK_H_
#define CHUNK_H_

#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/types.h>
#ifdef HAVE_ALLOCA_H
#include <alloca.h>
#endif

#include "utils/utils.h"

typedef struct chunk_t chunk_t;

/**
 * General purpose pointer/length abstraction.
 */
struct chunk_t {
	/** Pointer to start of data */
	unsigned char *ptr;
	/** Length of data in bytes */
	size_t len;
};

/**
 * A { NULL, 0 }-chunk handy for initialization.
 */
extern chunk_t chunk_empty;

/**
 * Create a new chunk pointing to "ptr" with length "len"
 */
inline chunk_t chunk_create(unsigned char *ptr, size_t len)
{
	chunk_t chunk = {ptr, len};
	return chunk;
}

/**
 * Create a clone of a chunk pointing to "ptr"
 */
chunk_t chunk_create_clone(unsigned char *ptr, chunk_t chunk);

/**
 * Calculate length of multiple chunks
 */
size_t chunk_length(const char *mode, ...);

/**
 * Concatenate chunks into a chunk pointing to "ptr".
 *
 * The mode string specifies the number of chunks, and how to handle each of
 * them with a single character: 'c' for copy (allocate new chunk), 'm' for move
 * (free given chunk) or 's' for sensitive-move (clear given chunk, then free).
 */
chunk_t chunk_create_cat(unsigned char *ptr, const char* mode, ...);

/**
 * Split up a chunk into parts, "mode" is a string of "a" (alloc),
 * "c" (copy) and "m" (move). Each letter say for the corresponding chunk if
 * it should get allocated on heap, copied into existing chunk, or the chunk
 * should point into "chunk". The length of each part is an argument before
 * each target chunk. E.g.:
 * chunk_split(chunk, "mcac", 3, &a, 7, &b, 5, &c, d.len, &d);
 */
void chunk_split(chunk_t chunk, const char *mode, ...);

/**
 * Write the binary contents of a chunk_t to a file
 *
 * @param chunk			contents to write to file
 * @param path			path where file is written to
 * @param label			label specifying file type
 * @param mask			file mode creation mask
 * @param force			overwrite existing file by force
 * @return				TRUE if write operation was successful
 */
bool chunk_write(chunk_t chunk, char *path, char *label, mode_t mask, bool force);

/**
 * Convert a chunk of data to hex encoding.
 *
 * The resulting string is '\\0' terminated, but the chunk does not include
 * the '\\0'. If buf is supplied, it must hold at least (chunk.len * 2 + 1).
 *
 * @param chunk			data to convert to hex encoding
 * @param buf			buffer to write to, NULL to malloc
 * @param uppercase		TRUE to use uppercase letters
 * @return				chunk of encoded data
 */
chunk_t chunk_to_hex(chunk_t chunk, char *buf, bool uppercase);

/**
 * Convert a hex encoded in a binary chunk.
 *
 * If buf is supplied, it must hold at least (hex.len / 2) + (hex.len % 2)
 * bytes. It is filled by the right to give correct values for short inputs.
 *
 * @param hex			hex encoded input data
 * @param buf			buffer to write decoded data, NULL to malloc
 * @return				converted data
 */
chunk_t chunk_from_hex(chunk_t hex, char *buf);

/**
 * Convert a chunk of data to its base64 encoding.
 *
 * The resulting string is '\\0' terminated, but the chunk does not include
 * the '\\0'. If buf is supplied, it must hold at least (chunk.len * 4 / 3 + 1).
 *
 * @param chunk			data to convert
 * @param buf			buffer to write to, NULL to malloc
 * @return				chunk of encoded data
 */
chunk_t chunk_to_base64(chunk_t chunk, char *buf);

/**
 * Convert a base64 in a binary chunk.
 *
 * If buf is supplied, it must hold at least (base64.len / 4 * 3).
 *
 * @param base64		base64 encoded input data
 * @param buf			buffer to write decoded data, NULL to malloc
 * @return				converted data
 */
chunk_t chunk_from_base64(chunk_t base64, char *buf);

/**
 * Convert a chunk of data to its base32 encoding.
 *
 * The resulting string is '\\0' terminated, but the chunk does not include
 * the '\\0'. If buf is supplied, it must hold (chunk.len * 8 / 5 + 1) bytes.
 *
 * @param chunk			data to convert
 * @param buf			buffer to write to, NULL to malloc
 * @return				chunk of encoded data
 */
chunk_t chunk_to_base32(chunk_t chunk, char *buf);

/**
 * Free contents of a chunk
 */
inline void chunk_free(chunk_t *chunk)
{
	free(chunk->ptr);
	*chunk = chunk_empty;
}

/**
 * Overwrite the contents of a chunk and free it
 */
inline void chunk_clear(chunk_t *chunk)
{
	if (chunk->ptr)
	{
		memwipe(chunk->ptr, chunk->len);
		chunk_free(chunk);
	}
}

/**
 * Initialize a chunk using a char array
 */
#define chunk_from_chars(...) ((chunk_t){(char[]){__VA_ARGS__}, sizeof((char[]){__VA_ARGS__})})

/**
 * Initialize a chunk to point to a thing
 */
#define chunk_from_thing(thing) chunk_create((char*)&(thing), sizeof(thing))

/**
 * Initialize a chunk from a string, not containing 0-terminator
 */
#define chunk_from_str(str) ({char *x = (str); chunk_create(x, strlen(x));})

/**
 * Allocate a chunk on the heap
 */
#define chunk_alloc(bytes) ({size_t x = (bytes); chunk_create(x ? malloc(x) : NULL, x);})

/**
 * Allocate a chunk on the stack
 */
#define chunk_alloca(bytes) ({size_t x = (bytes); chunk_create(x ? alloca(x) : NULL, x);})

/**
 * Clone a chunk on heap
 */
#define chunk_clone(chunk) ({chunk_t x = (chunk); chunk_create_clone(x.len ? malloc(x.len) : NULL, x);})

/**
 * Clone a chunk on stack
 */
#define chunk_clonea(chunk) ({chunk_t x = (chunk); chunk_create_clone(x.len ? alloca(x.len) : NULL, x);})

/**
 * Concatenate chunks into a chunk on heap
 */
#define chunk_cat(mode, ...) chunk_create_cat(malloc(chunk_length(mode, __VA_ARGS__)), mode, __VA_ARGS__)

/**
 * Concatenate chunks into a chunk on stack
 */
#define chunk_cata(mode, ...) chunk_create_cat(alloca(chunk_length(mode, __VA_ARGS__)), mode, __VA_ARGS__)

/**
 * Skip n bytes in chunk (forward pointer, shorten length)
 */
inline chunk_t chunk_skip(chunk_t chunk, size_t bytes)
{
	if (chunk.len > bytes)
	{
		chunk.ptr += bytes;
		chunk.len -= bytes;
		return chunk;
	}
	return chunk_empty;
}

/**
 * Skip a leading zero-valued byte
 */
inline chunk_t chunk_skip_zero(chunk_t chunk)
{
	if (chunk.len > 1 && *chunk.ptr == 0x00)
	{
		chunk.ptr++;
		chunk.len--;
	}
	return chunk;
}


/**
 *  Compare two chunks, returns zero if a equals b
 *  or negative/positive if a is small/greater than b
 */
int chunk_compare(chunk_t a, chunk_t b);

/**
 * Compare two chunks for equality,
 * NULL chunks are never equal.
 */
inline bool chunk_equals(chunk_t a, chunk_t b)
{
	return a.ptr != NULL  && b.ptr != NULL &&
			a.len == b.len && memeq(a.ptr, b.ptr, a.len);
}

/**
 * Compare two chunks (given as pointers) for equality (useful as callback),
 * NULL chunks are never equal.
 */
inline bool chunk_equals_ptr(chunk_t *a, chunk_t *b)
{
	return a != NULL && b != NULL && chunk_equals(*a, *b);
}

/**
 * Increment a chunk, as it would reprensent a network order integer.
 *
 * @param chunk			chunk to increment
 * @return				TRUE if an overflow occurred
 */
bool chunk_increment(chunk_t chunk);

/**
 * Check if a chunk has printable characters only.
 *
 * If sane is given, chunk is cloned into sane and all non printable characters
 * get replaced by "replace".
 *
 * @param chunk			chunk to check for printability
 * @param sane			pointer where sane version is allocated, or NULL
 * @param replace		character to use for replaceing unprintable characters
 * @return				TRUE if all characters in chunk are printable
 */
bool chunk_printable(chunk_t chunk, chunk_t *sane, char replace);

/**
 * Computes a 32 bit hash of the given chunk.
 *
 * @note The output of this function is randomized, that is, it will only
 * produce the same output for the same input when calling it from the same
 * process.  For a more predictable hash function use chunk_hash_static()
 * instead.
 *
 * @note This hash is only intended for hash tables not for cryptographic
 * purposes.
 *
 * @param chunk			data to hash
 * @return				hash value
 */
unsigned int chunk_hash(chunk_t chunk);

/**
 * Incremental version of chunk_hash. Use this to hash two or more chunks.
 *
 * @param chunk			data to hash
 * @param hash			previous hash value
 * @return				hash value
 */
unsigned int chunk_hash_inc(chunk_t chunk, unsigned int hash);

/**
 * Computes a 32 bit hash of the given chunk.
 *
 * Compared to chunk_hash() this will always calculate the same output for the
 * same input.  Therefore, it should not be used for hash tables (to prevent
 * hash flooding).
 *
 * @note This hash is not intended for cryptographic purposes.
 *
 * @param chunk			data to hash
 * @return				hash value
 */
unsigned int chunk_hash_static(chunk_t chunk);

/**
 * Incremental version of chunk_hash_static(). Use this to hash two or more
 * chunks in a predictable way.
 *
 * @param chunk			data to hash
 * @param hash			previous hash value
 * @return				hash value
 */
unsigned int chunk_hash_static_inc(chunk_t chunk, unsigned int hash);

/**
 * Computes a quick MAC from the given chunk and key using SipHash.
 *
 * The key must have a length of 128-bit (16 bytes).
 *
 * @note While SipHash has strong features using it for cryptographic purposes
 * is not recommended (in particular because of the rather short output size).
 *
 * @param chunk			data to process
 * @param key			key to use
 * @return				MAC for given input and key
 */
unsigned long int chunk_mac(chunk_t chunk, unsigned char *key);

#endif /** CHUNK_H_ @}*/
