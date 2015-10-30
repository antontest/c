/**
 * @defgroup debug debug
 * @{ @ingroup utils
 */

#ifndef DEBUG_H_
#define DEBUG_H_

typedef enum debug_t debug_t;
typedef enum level_t level_t;

#include <stdio.h>
#include "utils/enum.h"

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
	LEVEL_POS = -1,
	/** most important auditing logs */
	LEVEL_AUDIT ,
	/** control flow */
	LEVEL_CTRL,
	/** diagnose problems */
	LEVEL_DIAG,
	/** raw binary blobs */
	LEVEL_RAW,
	/** including sensitive data (private keys) */
	LEVEL_PRIVATE,
};

#ifndef DEBUG_LEVEL
# define DEBUG_LEVEL 4
#endif /* DEBUG_LEVEL */

/** debug macros, they call the dbg function hook */
#if DEBUG_LEVEL >= 0
# define DBG0(group, fmt, ...) dbg(group, 0, fmt, ##__VA_ARGS__)
#endif /* DEBUG_LEVEL */
#if DEBUG_LEVEL >= 1
# define DBG1(group, fmt, ...) dbg(group, 1, fmt, ##__VA_ARGS__)
#endif /* DEBUG_LEVEL */
#if DEBUG_LEVEL >= 2
# define DBG2(group, fmt, ...) dbg(group, 2, fmt, ##__VA_ARGS__)
#endif /* DEBUG_LEVEL */
#if DEBUG_LEVEL >= 3
# define DBG3(group, fmt, ...) dbg(group, 3, fmt, ##__VA_ARGS__)
#endif /* DEBUG_LEVEL */
#if DEBUG_LEVEL >= 4
# define DBG4(group, fmt, ...) dbg(group, 4, fmt, ##__VA_ARGS__)
#endif /* DEBUG_LEVEL */

#ifndef DBG0
# define DBG0(...) {}
#endif
#ifndef DBG1
# define DBG1(...) {}
#endif
#ifndef DBG2
# define DBG2(...) {}
#endif
#ifndef DBG3
# define DBG3(...) {}
#endif
#ifndef DBG4
# define DBG4(...) {}
#endif

/** dbg function hook, uses dbg_default() by default */
extern void (*dbg) (debug_t group, level_t level, char *fmt, ...);

/** default logging function */
void dbg_default(debug_t group, level_t level, char *fmt, ...);

/** set the level logged by dbg_default() */
void dbg_default_set_level(level_t level);

/** set the stream logged by dbg_default() to */
void dbg_default_set_stream(FILE *stream);

#endif /** DEBUG_H_ @}*/

