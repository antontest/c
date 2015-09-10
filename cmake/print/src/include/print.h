/**
 * usual head files
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>

#ifndef debug_error
#define debug_error(...) \
    do { \
        fprintf(stderr, "\033[1;31m[%s() %d] \033[0m", \
                __func__, __LINE__); \
        fprintf(stderr, ##__VA_ARGS__); \
        fprintf(stderr, "\n"); \
    } while(0);
#endif

#ifndef debug_info
#define debug_info(...) \
    do { \
        fprintf(stdout, "\033[1;35m[%s() %d] \033[0m", \
                __func__, __LINE__); \
        fprintf(stdout, ##__VA_ARGS__); \
        fprintf(stdout, "\n\033[0;0m"); \
    } while(0); 
#endif

#ifndef color
#define color(color_num) "\033[0;"#color_num"m"
#endif
#ifndef color_end
#define color_end "\033[0m"
#endif
