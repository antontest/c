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
 * @param pid  [out] process pids
 * @param size [in]  size of buffer
 *
 * @return pid , if succ; -1, if failed.
 */
int get_pid(const char *proc_name, int pid[], int size);

/**
 * @brief get process name with pid
 *
 * @param pid   [in]  id of process
 * @param name  [out] buffer of process name
 * @param size  [out] size of buffer
 *
 * @return 0, if succ; -1, if failed
 */
int get_proc_name(pid_t pid, char *name, int size);

/**
 * @brief check process is unique 
 *
 * @param name [in] process name
 *
 * @return 1, if process unique; otherwise return 0
 */
int check_proc_unique(const char *proc_name);

/**
 * @brief get proc exe path by pid
 *
 * @param pid  [in]  process id
 * @param path [out] exe path of process
 * @param size [in] size of buffer
 *
 * @return 0, if succ; -1, if failed
 */
int get_exec_path(pid_t pid, char *path, int size);

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

/**
 * @brief SYSTEM 
 *
 * @param format
 * @param ...
 *
 * @return id of process, if succ; -1, if failed
 */
int SYSTEM(const char *format, ...);

/**
 * @brief read system uptime
 *
 * @return uptime
 */
int read_uptime(void);

/**
 * @brief create_state_file_by_name 
 *
 * @param app_name [in] process's name
 *
 * @return 0, if succ; -1, if failed
 */
int create_state_file_by_name(const char *app_name);

/**
 * @brief unlink_state_file_by_name 
 *
 * @param app_name [in] process's name
 */
void unlink_state_file_by_name(const char *app_name);

/**
 * @brief is_state_file_exist 
 *
 * @param app_name [in] app_name
 *
 * @return 0, if state file exist; -1, if not exist
 */
int is_state_file_exist(const char *app_name);

/**
 * @brief This function is used ti check the app command can be call legal.
 *        It avoid that two or more faimiliar command is called at the same time.
 *
 * @param app_name    [in] app name
 * @param state_file  [in] path of app state file
 * @param app_stop_cb [in] callback function of app stopping
 *
 * @return 0, if only one app; 1, if two or more
 */
int check_app_start_conflict(const char *app_name, const char *state_file, int (*app_stop_cb)(void));

/**
 * @brief It's a public interface for the app, which can check app sttaus
 *
 * @param app_name             [in] app name
 * @param pid_num              [in] count of app running
 * @param uptime               [in] uptime of app
 * @param state_file           [in] path of app state file
 * @param app_state_check_cb   [in] callback of app state checking
 *
 * @return app state
 */
int app_state_check(const char *app_name, int pid_num, int uptime, const char *state_file, int (*app_state_check_cb)(void));

#endif
