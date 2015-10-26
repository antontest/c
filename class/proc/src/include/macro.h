
/*********************************************************
 *****************    Macro Defination    ****************
 *********************************************************/
#ifndef macro_to_str
#define macro_to_str(x) #x
#endif

#ifndef macro_cat
#define macro_cat(a, b) a##b
#endif

#ifndef func_name
#define func_name __func__
#endif

#ifndef line_num
#define line_num __LINE__
#endif

#ifndef debug_error
#define debug_error(...) \
    do { \
        fprintf(stderr, "\033[1;35m[ Function %s ] [ line %d ] \033[0m", \
                func_name, line_num); \
        fprintf(stderr, ##__VA_ARGS__); \
        fprintf(stderr, "\n"); \
    } while(0);
#endif

#ifndef debug_info
#define debug_info(...) \
    do { \
        fprintf(stdout, "\033[1;35m"); \
        fprintf(stdout, ##__VA_ARGS__); \
        fprintf(stdout, "\n\033[0;0m"); \
    } while(0);
#endif

#ifndef offset_of
#define offset_of(type, member) ((unsigned int)&(((type *)0)->member))
#endif

#ifndef container_of
#define container_of(ptr, type, member) ({\
    const typeof(((type *)0)->member) *_mptr = ptr; \
    (type *)((char *)_mptr - offset_of(type, member));})
#endif

#ifndef swap
#define swap(a, b) { a = a ^ b; b = a ^ b; a = a ^ b; }
#endif

#ifndef arr_size
#define arr_size(a) ( sizeof(a) / sizeof(a[0]) )
#endif

