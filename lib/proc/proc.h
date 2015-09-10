#ifndef __PROC_H__
#define __PROC_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <stdarg.h>
#include <fcntl.h>

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

/**
 * @brief get value of shell return
 *
 * @param cmd    [in]  shell command
 * @param result [out] return value
 * @param size   [in]  size of result buffer
 *
 * @return 0, if succ; -1, if failed.
 */
int get_shell_result(const char *cmd, char *result, int size);

/**
 * @brief get parent id of current process with id
 *
 * @param pid [in] id of current process
 *
 * @return parent id, if succ; -1, if failed.
 */
int get_ppid(pid_t pid);

/**
 * @brief get process id by process name
 *
 * @param name [in] process name
 *
 * @return pid , if succ; -1, if failed.
 */
int get_pid_by_name(const char *proc_name);

/**
 * @brief get process id by process name
 *
 * @param name [in] process name
 *
 * @return pid , if succ; -1, if failed.
 */
int get_id_by_name(const char *name);

/**
 * @brief get process name by pid
 *
 * @param pid        [in]  process id
 * @param proc_name  [out] process name
 * @param size       [in]  buffer size of proc_name
 *
 * @return 0, if succ; -1, if failed.
 */
int get_proc_name_by_pid(pid_t pid, char *proc_name, int size);

/**
 * @brief get process name with pid
 *
 * @param pid   [in]  id of process
 * @param name  [out] buffer of process name
 * @param size  [out] size of buffer
 *
 * @return 0, if succ; -1, if failed
 */
int get_name_by_pid(pid_t pid, char *name, int size);

/**
 * @brief check_proc_unique 
 *
 * @param name [in] process name
 *
 * @return 1, if process unique; otherwise return 0
 */
int check_proc_unique(const char *name);


/**
 * @brief check process is unique 
 *
 * @param name [in] process name
 *
 * @return 1, if process unique; otherwise return 0
 */
int is_proc_unique(const char *proc_name);

/**
 * @brief get proc exe path by pid
 *
 * @param pid  [in]  process id
 * @param path [out] exe path of process
 * @param size [in] size of buffer
 *
 * @return 0, if succ; -1, if failed
 */
int get_exe_path_by_pid(pid_t pid, char *path, int size);

/**
 * @brief get file path by fd and pid
 *
 * @param fd   [in]  fd of file opened.
 * @param pid  [in]  process id
 * @param path [out] exe path of process
 * @param size [in] size of buffer
 *
 * @return 0, if succ; -1, if failed
 */
int get_file_path(int fd, int pid, char *path, int size);

/**
 * @brief get cmd exe line
 *
 * @param pid     [in]  pid
 * @param cmdline [out] cmd line
 * @param size    [in]  size of buffer
 *
 * @return 0, if succ; -1, failed.
 */
int get_cmdline(int pid, char *cmdline, int size);

#endif
