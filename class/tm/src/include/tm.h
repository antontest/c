#ifndef __TM_H__
#define __TM_H__
#include <time.h>

/**
 * @brief get system real current time
 */
struct tm *real_sys_time();

/**
 * @brief run_time_start 
 */
void run_time_start();

/**
 * @brief run_time_end 
 */
struct timespec *run_time_end();

#endif /* __TM_H__ */
