/**
 * @defgroup debug debug
 * @{ @ingroup utils
 */

#ifndef DEBUG_H_
#define DEBUG_H_

typedef enum debug_t debug_t;
typedef enum level_t level_t;

#include <stdio.h>
#include "enum.h"

/**
 * Debug message group.
 */
enum debug_t {
        /** no dedug module info */
        DBG_NULL = -1,
	/** daemon specific */
	DBG_DMN = 1,
	/** utils */
	DBG_UTL,
	/** timer */
	DBG_TMR,
	/** thread */
	DBG_THR,
	/** socket */
	DBG_NET,
	/** configuration backends */
	DBG_CFG,
	/** process */
	DBG_PRO,
	/** directory */
	DBG_DIR,
	/** debugger */
	DBG_DBG,
	/** logger */
	DBG_LOG,
	/** applications other than daemons */
	DBG_APP,
	/** libstrongswan */
	DBG_LIB,
	/** number of groups */
	DBG_MAX,
	/** pseudo group with all groups */
	DBG_ANY = DBG_MAX,
};

/**
 * Debug levels used to control output verbosity.
 */
enum level_t {
	/** absolutely silent */
	LEVEL_SILENT = -1,
	/** log postion in file */
	LEVEL_DEBUG = 1,
	/** most important auditing logs */
	LEVEL_TRACE,
	/** control flow */
	LEVEL_NOTICE,
	/** diagnose problems */
	LEVEL_WARNING,
	/** raw binary blobs */
	LEVEL_ERROR,
	/** including sensitive data (private keys) */
	LEVEL_PRIVATE,
};


#ifndef DEBUG_LEVEL
# define DEBUG_LEVEL 5
#endif /* DEBUG_LEVEL */

/** debug macros, they call the dbg function hook */
#if DEBUG_LEVEL >= 1
#define dbg(fmt, ...) \
    do { \
        _dbg(stdout, -1, -1, fmt, ##__VA_ARGS__); \
    } while (0)
#endif /* DEBUG_LEVEL */
//dbg(stdout, group, 1, "[%s:%d][%s] "fmt, __FILE__, __LINE__,  __func__, ##__VA_ARGS__); 
#if DEBUG_LEVEL >= 1
#define dbg_pos(fmt, ...) \
    do { \
        char *file_name = strrchr(__FILE__, '/'); \
        _dbg(stdout, -1, -1, "[%s:%d][%s] " fmt , \
        file_name + 1, __LINE__, __func__, ##__VA_ARGS__); \
    } while (0)
#endif /* DEBUG_LEVEL */
#if DEBUG_LEVEL >= 5
#define dbg_err(group, fmt, ...) \
    do { \
        char *file_name = strrchr(__FILE__, '/'); \
        _dbg(stderr, group, LEVEL_ERROR, "[%s:%d][%s] " fmt , \
        file_name + 1, __LINE__, __func__, ##__VA_ARGS__); \
    } while (0)
#endif /* DEBUG_LEVEL */

#define dbg0(fmt, ...) \
    do { \
        _dbg(stdout, -1, -1, fmt, ##__VA_ARGS__); \
    } while (0)
#define dbg1(fmt, ...) \
    do { \
        char *file_name = strrchr(__FILE__, '/'); \
        _dbg(stdout, -1, -1, "[%s:%d][%s] " fmt , \
        file_name + 1, __LINE__, __func__, ##__VA_ARGS__); \
    } while (0)
#define dbg2(fmt, ...) \
    do { \
        char *file_name = strrchr(__FILE__, '/'); \
        _dbg(stdout, DBG_DBG, -1, "[%s:%d][%s] " fmt , \
        file_name + 1, __LINE__, __func__, ##__VA_ARGS__); \
    } while (0)

#ifndef dbg_debug
# define dbg_debug(...) {}
#endif
#ifndef dbg_pos
# define dbg_pos(...) {}
#endif
#ifndef dbg_err
# define dbg_err(...) {}
#endif

/** dbg function hook, uses dbg_default() by default */
extern void (*_dbg) (FILE *stream, debug_t group, level_t level, char *fmt, ...);

/** default logging function */
void dbg_default(FILE *stream, debug_t group, level_t level, char *fmt, ...);

/** set the level logged by dbg_default() */
void dbg_default_set_level(level_t level);

/** set the stream logged by dbg_default() to */
void dbg_default_set_stream(FILE *stream);

#endif /** DEBUG_H_ @}*/

