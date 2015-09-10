#include <stdio.h>

/**
 * @brief init fob log
 *
 * @param path [in] path of log file
 *
 * @return 0, if succ; -1, if failed
 */
int fib_log_init(const char *path);

/**
 * @brief free file
 */
void fib_log_deinit();

/**
 * @brief write log into log file
 *
 * @param str [in] log info
 *
 * @return 0, if succ; -1, if failed
 */
int fib_log_write(const char *str);
