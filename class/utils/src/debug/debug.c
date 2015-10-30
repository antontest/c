#include <stdarg.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <sys/utsname.h>
#include "debug.h"
#include <utils/utils.h>

ENUM(debug_names, DBG_DMN, DBG_LIB,
        "DMN",
        "UTL",
        "TMR",
        "THR",
        "NET",
        "CFG",
        "PRO",
        "DIR",
        "DBG",
        "LOG",
        "APP",
        "LIB",
);

ENUM(debug_lower_names, DBG_DMN, DBG_LIB,
        "dmn",
        "utl",
        "tmr",
        "thr",
        "net",
        "cfg",
        "pro",
        "dir",
        "dbg",
        "log",
        "app",
        "lib",
);

ENUM(level_names, LEVEL_DEBUG, LEVEL_PRIVATE,
        "DEBUG",
        "TRACE",
        "NOTICE",
        "WARNING",
        "ERROR",
        "PRIVATE",
);

ENUM(level_lower_names, LEVEL_DEBUG, LEVEL_PRIVATE,
        "debug",
        "trace",
        "notice",
        "warning",
        "error",
        "private",
);

#define LOG_PATH_LEN     (250)
#define DBG_BUFF_LEN (1024 * 1024 * 4)

/**
 * level logged by the default logger
 */
static level_t default_level = 1;

/**
 * stream logged to by the default logger
 */
static FILE *default_stream = NULL;

static char debug_char[DBG_BUFF_LEN] = {0};
static int  debug_buff_used_len = 0;

/**
 * time string
 */
static char *log_time_str()
{
    struct tm *ptr = NULL;
    time_t lt;

    lt  = time(NULL);
    ptr = localtime(&lt);
    debug_buff_used_len += strftime(debug_char, DBG_BUFF_LEN - debug_buff_used_len, "%b %d %Y %T", ptr);

    return debug_char;
}

/**
 * Make log head string with system type
 */
static void log_system_type_str()
{
    struct  utsname u;
    if (uname(&u) != -1) {
        debug_buff_used_len += sprintf(debug_char + debug_buff_used_len, " %s", u.sysname);
    }
}

/**
 * Make log head string with process name
 */
static void log_proc_name_str()
{
    char *proc_name = NULL;
    char proc_path[LOG_PATH_LEN] = {0};
    
    ignore_result(readlink("/proc/self/exe", proc_path, LOG_PATH_LEN));
    proc_name = strrchr(proc_path, '/');
    if (proc_name != NULL) {
        debug_buff_used_len += sprintf(debug_char + debug_buff_used_len, " %s", proc_name + 1);
    }
}

/**
 * default dbg function which printf all to stderr
 */
void dbg_default(FILE *stream, debug_t group, level_t level, char *fmt, ...)
{
    if (!default_stream)
        default_stream = stderr;

    va_list args;
    debug_buff_used_len = 0;

    log_time_str();
    log_system_type_str();
    log_proc_name_str();
    if (group > 0 || level > 0) {
        strcat(debug_char, "[");
        debug_buff_used_len += 1;
    }
    if(group > 0)
        debug_buff_used_len += sprintf(debug_char + debug_buff_used_len, "%s", enum_to_name(debug_names, group));
    if (level > 0) {
        if (group > 0) {
            strcat(debug_char, " ");
            debug_buff_used_len += 1;
        }
        debug_buff_used_len += sprintf(debug_char + debug_buff_used_len, "%s", enum_to_name(level_names, level));
    }
    if (group > 0 || level > 0) {
        strcat(debug_char, "]");
        debug_buff_used_len += 1;
    }
    strcat(debug_char, ": ");
    debug_buff_used_len += 2;

    va_start(args, fmt);
    vsnprintf(debug_char + debug_buff_used_len, DBG_BUFF_LEN - debug_buff_used_len, fmt, args);
    va_end(args);
    fprintf(default_stream, "%s\n", debug_char);
    //fflush(default_stream);
}

/**
 * set the level logged by the default stderr logger
 */
void dbg_default_set_level(level_t level)
{
    default_level = level;
}

/**
 * set the stream logged by dbg_default() to
 */
void dbg_default_set_stream(FILE *stream)
{
    default_stream = stream;
}

/**
 * The registered debug hook.
 */
void (*_dbg) (FILE *stream, debug_t group, level_t level, char *fmt, ...) = dbg_default;
