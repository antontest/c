#include <log.h>
#include <utils.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <sys/utsname.h>
#include <stdarg.h>

#define LOG_PATH_LEN     (250)
#define LOG_BUFF_LEN     (1024 * 1024 * 4)
#define SYS_LOG_BUFF_LEN (1024 * 1024 * 8)

ENUM(debug_name, DBG_DMN, DBG_LIB,
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

ENUM(debug_lower_name, DBG_DMN, DBG_LIB,
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

ENUM(level_name, LEVEL_DEBUG, LEVEL_PRIVATE,
        "DEBUG",
        "TRACE",
        "NOTICE",
        "WARNING",
        "ERROR",
        "PRIVATE",
);

ENUM(level_lower_name, LEVEL_DEBUG, LEVEL_PRIVATE,
        "debug",
        "trace",
        "notice",
        "warning",
        "error",
        "private",
);

typedef struct private_log_t private_log_t;

struct private_log_t  {
    /**
     * Public part of a log object.
     */
    log_t public;

    /**
     * Log file path
     */
    char log_path[LOG_PATH_LEN];

    /**
     * Log buffer
     */
    char log_buff[LOG_BUFF_LEN];

    /**
     * Stream logged to by the default logger
     */
    FILE *default_stream;

    /**
     * debug group logged by the default logger
     */
    debug_t default_group;

    /**
     * level logged by the default logger
     */
    level_t default_level;
};

/**
 * Make log head string with system time info
 */
static int log_time_str(char *log_buff)
{
    struct tm *ptr = NULL;
    time_t lt;

    lt  = time(NULL);
    ptr = localtime(&lt);
    return strftime(log_buff, LOG_BUFF_LEN, "%b %d %Y %T", ptr);
}

/**
 * Make log head string with system type
 */
static int log_system_type_str(char *log_buff)
{
    struct  utsname u;
    if (uname(&u) != -1) {
        return sprintf(log_buff, "%s", u.sysname);
    }

    return 0;
}


/**
 * Make log head string with process name
 */
static int log_proc_name_str(char *log_buff)
{
    char *proc_name = NULL;
    char proc_path[LOG_PATH_LEN] = {0};
    
    ignore_result(readlink("/proc/self/exe", proc_path, LOG_PATH_LEN));
    proc_name = strrchr(proc_path, '/');
    if (proc_name != NULL) {
        return sprintf(log_buff, "%s", proc_name + 1);
    }

    return 0;
}

METHOD(log_t, get_level, int, private_log_t *this)
{
    return this->default_level;
}

METHOD(log_t, log_, void, private_log_t *this, debug_t group, level_t level, char *fmt, ...)
{
    va_list args;
    int buff_use_len = 0;
    char *log_buff_ptr = this->log_buff;

    if (!this->default_stream) {
        this->default_stream = stdout;
    }

    if (level < this->default_level) return ;
    buff_use_len = log_time_str(log_buff_ptr);
    strcat(log_buff_ptr, " ");
    log_buff_ptr += buff_use_len + 1;

    buff_use_len = log_system_type_str(log_buff_ptr);
    strcat(log_buff_ptr, " ");
    log_buff_ptr += buff_use_len + 1;

    buff_use_len = log_proc_name_str(log_buff_ptr);
    log_buff_ptr += buff_use_len;

    buff_use_len = sprintf(log_buff_ptr, "[%s %s]: ", enum_to_name(debug_name, group), enum_to_name(level_name, level));
    log_buff_ptr += buff_use_len;

    va_start(args, fmt);
    vsnprintf(log_buff_ptr, LOG_BUFF_LEN, fmt, args);
    va_end(args);
    fprintf(this->default_stream, "%s\n", this->log_buff);
    fflush(this->default_stream);
}

METHOD(log_t, destroy_, void, private_log_t *this)
{
    fflush(this->default_stream);
    if (this->default_stream != stdout && this->default_stream != stderr) {
        free(this->default_stream);
    }
    this->default_stream = NULL;

    free(this);
    this = NULL;
}

/**
 * Create the log instance
 */
log_t *log_create(const char *log_file)
{
    private_log_t *this;

    INIT(this,
        .public = {
        .log = _log_,
        .get_level = _get_level,
        .destroy = _destroy_,
        },
        .default_group  = -1,
        .default_level  = LEVEL_DEBUG,
    );

    if (!log_file) goto ret;
    this->default_stream = fopen(log_file, "a+");

ret:
    return &this->public;
}

struct log_t *default_log = NULL;

/**
 * Create the default log instance
 */
void log_init(const char *log_file)
{
    default_log = log_create(log_file);
}

/**
 * Free the default log instance.
 */
void log_deinit()
{
    if (!default_log) return;
    default_log->destroy(default_log);
}
