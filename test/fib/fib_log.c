#include "fib_log.h"

FILE *fp = NULL;

/**
 * @brief init fob log
 *
 * @param path [in] path of log file
 *
 * @return 0, if succ; -1, if failed
 */
int fib_log_init(const char *path)
{
    if (path == NULL) return -1;
    if ((fp = fopen(path, "w")) == NULL) return -1;
   
    return 0;
}

/**
 * @brief free file
 */
void fib_log_deinit()
{
    if (fp != NULL) fclose(fp);
    fp = NULL;

    return ; 
}

/**
 * @brief write log into log file
 *
 * @param str [in] log info
 *
 * @return 0, if succ; -1, if failed
 */
int fib_log_write(const char *str)
{
    if (!fputs(str, fp)) return 0;

    return -1;
}
