#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdarg.h>

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

/*********************************************************
 **************    Function Declaration    ***************
 *********************************************************/
/**
 * @brief execl shell script
 *
 * @param cmd  [in]  shell command
 * @param pid  [out] process id of shell command
 *
 * @return 0, if succ; -1, if failed.
 */
int cmd_exec(const char *cmd, int *pid);

/**
 * @brief get value of shell return
 *
 * @param cmd    [in]  shell command
 * @param result [out] return value
 * @param size   [in]  size of result buffer
 *
 * @return 0, if succ; -1, if failed.
 */
int get_shell_return(const char *cmd, char *result, int size);

int check_proc_is_running(pid_t pid)
{
    if (0 == kill(pid, 0)) return 1;
    return 0;
}

struct st
{
    char a;
    int b;
    void *p;
};

#define t(cmd) "echo " #cmd " test"

/*********************************************************
 ******************    Main Function    ******************
 *********************************************************/
int main(int agrc, char *agrv[])
{
    int rt = 0; /* return value of function main */

    int a = 10, b = 100;
    char buf[256] = {0};
    swap(a, b);

    cmd_exec("ls", NULL);
    get_shell_return("ls | grep tags", buf, sizeof(buf));
    printf("result: %s", buf);

    printf("%s\n", t("#g#"));

    if (check_proc_is_running(getpid())) printf("running\n");

    sleep (100);
    return rt;
}

/**
 * @brief execl shell script
 *
 * @param cmd  [in]  shell command
 * @param pid  [out] process id of shell command
 *
 * @return 0, if succ; -1, if failed.
 */
int cmd_exec(const char *cmd, int *pid)
{
    pid_t p_id = -1;
    int rt = -1; 

    if (cmd == NULL) goto ret;

    if((p_id = fork()) < 0) goto ret;
    else if (p_id == 0)
    {
        setpgrp();
        if (execl("/bin/sh", "sh", "-c", cmd, (char *)0))
        {
            debug_info("execl %s failed.", cmd);
            goto ret;
        }
    }
    else
    {
        if (pid != NULL) *pid = p_id;

        while (waitpid(p_id, &rt, 0) < 0)
        {
            if (errno != EINTR) goto ret;
        }

        rt = 0;
    }

ret:
    return rt;
}

/**
 * @brief get value of shell return
 *
 * @param cmd    [in]  shell command
 * @param result [out] return value
 * @param size   [in]  size of result buffer
 *
 * @return 0, if succ; -1, if failed.
 */
int get_shell_return(const char *cmd, char *result, int size)
{
    FILE *fp = NULL;
    char buf[128] = {0};

    if (cmd == NULL) return -1;
    if ((fp = popen(cmd, "r")) == NULL) return -1;

    strcpy(result, "\0");
    while (fgets(buf, sizeof(buf), fp) != NULL)
    {
        if (result != NULL && (strlen(result) + strlen(buf)) < size) 
            strcat(result, buf);
    }
    if (fp != NULL) pclose(fp);

    return 0;
}

