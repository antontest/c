#ifndef __LOG_H__
#define __LOG_H__
#include "debug.h"

typedef struct log_t log_t;

struct log_t {
    /**
     * The format string specifies an additional informational or error
     * message with a printf() like variable argument list.
     * Use the DBG() macros. 
     *
     * @param group  debugging group
     * @param level  verbosity level of the signal
     * @param fmt    printf() style format string
     * @param ...    printf() style argument list
     */
    void (*log) (log_t *this, debug_t group, level_t level, char *fmt, ...);

    /**
     * @brief gain log current level
     */
    int (*get_level) (log_t *this);

    /**
     * @brief Free this log instance 
     */
    void (*destroy) (log_t *this);
};

/**
 * Create the log instance.
 */
log_t *log_create(const char *log_file);

/**
 * Create the log instance.
 */
int log_init(const char *log_file);

/**
 * Free the log instance.
 */
void log_deinit();

extern log_t *default_log;

#define log_debug(group, log_fmt, ...) \
    do { \
        char *file_name = strrchr(__FILE__, '/'); \
        default_log->log(default_log, group, LEVEL_DEBUG, "[%s:%d][%s] " log_fmt , \
        file_name + 1, __LINE__, __func__, ##__VA_ARGS__); \
    } while (0)

#define log_debug0(group, log_fmt, ...) \
    do { \
        default_log->log(default_log, group, LEVEL_DEBUG, log_fmt, ##__VA_ARGS__); \
    } while (0)

#define log_warn(group, log_fmt, ...) \
    do { \
        char *file_name = strrchr(__FILE__, '/'); \
        default_log->log(default_log, group, LEVEL_WARNING, "[%s:%d][%s] " log_fmt , \
        file_name + 1, __LINE__, __func__, ##__VA_ARGS__); \
    } while (0)


#define log_trace(group, log_fmt, ...) \
    do { \
        char *file_name = strrchr(__FILE__, '/'); \
        default_log->log(default_log, group, LEVEL_TRACE, "[%s:%d][%s] " log_fmt , \
        file_name + 1, __LINE__, __func__, ##__VA_ARGS__); \
    } while (0)

#define log_error(group, log_fmt, ...) \
    do { \
        char *file_name = strrchr(__FILE__, '/'); \
        default_log->log(default_log, group, LEVEL_ERROR, "[%s:%d][%s] " log_fmt , \
        file_name + 1, __LINE__, __func__, ##__VA_ARGS__); \
    } while (0)

#define log_notice(group, log_fmt, ...) \
    do { \
        char *file_name = strrchr(__FILE__, '/'); \
        default_log->log(default_log, group, LEVEL_NOTICE, "[%s:%d][%s] " log_fmt , \
        file_name + 1, __LINE__, __func__, ##__VA_ARGS__); \
    } while (0)

#define log_notice0(group, log_fmt, ...) \
    do { \
        default_log->log(default_log, group, LEVEL_NOTICE, log_fmt , ##__VA_ARGS__); \
    } while (0)

#define log_private(group, log_fmt, ...) \
    do { \
        char *file_name = strrchr(__FILE__, '/'); \
        default_log->log(default_log, group, LEVEL_PRIVATE, "[%s:%d][%s] " log_fmt , \
        file_name + 1, __LINE__, __func__, ##__VA_ARGS__); \
    } while (0)

#endif /* __LOG_H__ */
