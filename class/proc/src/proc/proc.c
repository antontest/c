#include "proc.h"

#define APP_STATE_FILE_PATH "/var/run/app_state"

typedef enum {
    APP_RUNNING = 0,
    APP_CRASHED,
    APP_STOPPED,
    APP_STARTING,
    APP_TIMEOUT
} app_state_t;

/**
 * @brief execl shell script
 *
 * @param cmd  [in]  shell command
 * @param pid  [out] process id of shell command
 *
 * @return id of process, if succ; -1, if failed.
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
            fprintf(stderr, "execl %s failed.", cmd);
            goto ret;
        }
        _exit(127);
    }
    else
    {
        if (pid != NULL) *pid = p_id;

        while (waitpid(p_id, &rt, 0) < 0)
        {
            if (errno != EINTR) goto ret;
        }
    }

ret:
    return p_id;
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
        else break;
    }
    if (fp != NULL) pclose(fp);

    return 0;
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
int get_shell_result(const char *cmd, char *result, int size)
{
    int fd[2] = {0};
    int status = -1;
    int n = 0;
    int count = 0;
    pid_t pid = -1;
    char buf[256] = {0};

    if (cmd == NULL || result == NULL || size <= 0) return -1;
    if (pipe(fd) < 0) return -1;

    strcpy(result, "\0");
    pid = fork();
    if (pid == -1) status = -1;
    else if (pid == 0) 
    {
        close(fd[0]);
        if (fd[1] != STDOUT_FILENO)
        {
            if (dup2(fd[1], STDOUT_FILENO) != STDOUT_FILENO)
                _exit(127);
            close(fd[1]);
        }
        execl("/bin/sh", "sh", "-c", cmd, (char *)0);
        _exit(127);
    }
    else
    {
        close(fd[1]);
        while ((n = read(fd[0], buf, sizeof(buf))) > 0)
        {
            count += n;
            if (count < size) strcat(result, buf);
        }
        close(fd[0]);

        while (waitpid(pid, &status, 0) < 0)
        {
            if (errno != EINTR)
                break;
        }
        status = 0;
    }

    return status;
}

/**
 * @brief get parent id of current process with id
 *
 * @param pid [in] id of current process
 *
 * @return parent id, if succ; -1, if failed.
 */
int get_ppid(pid_t pid)
{
    char path[64] = {0};
    char buf[128] = {0};
    FILE *fp = NULL;
    pid_t ppid = -1;

    sprintf(path, "/proc/%d/status", pid);
    if ((fp = fopen(path, "r")) == NULL) return -1;

    while (fgets(buf, sizeof(buf), fp) != NULL)
    {
        if (strstr(buf, "Pid") != NULL)
            break;
    }

    if (fscanf(fp, "PPid:%d", &ppid) == -1) ppid = -1;
    if (fp != NULL) fclose(fp);

    return ppid;
}

/**
 * @brief get process id by process name
 *
 * @param name [in]  process name
 * @param pid  [out] process pids
 * @param size [in]  size of buffer
 *
 * @return pid , if succ; -1, if failed.
 */
int get_pid(const char *proc_name, int pid[], int size)
{
    struct dirent *dir = NULL;
    DIR *dirp = NULL;
    FILE *fp = NULL;
    char name[64] = {0};
    char path[128] = {0};
    int pid_num = 0;

    if (pid == NULL) return -1;
    if ((dirp = opendir("/proc/")) == NULL) return -1;

    while ((dir = readdir(dirp)) != NULL)
    {
        if (dir->d_name && (atoi(dir->d_name) > 0))
        {
            sprintf(path, "/proc/%s/status", dir->d_name);
            if ((fp = fopen(path, "r")) == NULL) continue;
            if (fscanf(fp, "Name:%s", name) == -1) 
                name[0] = '\0';
            if (fp != NULL) fclose(fp);
            if (strcmp(name, proc_name)) continue;
            pid[pid_num++] = atoi(dir->d_name);
            if (pid_num >= size) {
                break;
            }
        }
    }

    if (dirp != NULL) closedir(dirp);

    return 0;
}

/**
 * @brief get process name with pid
 *
 * @param pid   [in]  id of process
 * @param name  [out] buffer of process name
 * @param size  [out] size of buffer
 *
 * @return 0, if succ; -1, if failed
 */
int get_proc_name(pid_t pid, char *name, int size)
{
    char path[64] = {0};
    char buf[64] = {0};
    FILE *fp = NULL;
    int rt = -1;

    if (name == NULL) return -1;
    sprintf(path, "/proc/%d/status", pid);
    if ((fp = fopen(path, "r")) == NULL) goto free;

    if (fscanf(fp, "Name: %s", buf) == -1) goto free;
    strncpy(name, buf, size);
    rt = 0;

free:
    if (fp != NULL) fclose(fp);

    return rt;
}

/**
 * @brief check process is unique 
 *
 * @param name [in] process name
 *
 * @return pid of already running, if process unique;  
 *          otherwise return 0
 */
int check_proc_unique(const char *proc_name)
{
    struct dirent *dir = NULL;
    DIR *dirp = NULL;
    FILE *fp = NULL;
    char name[64] = {0};
    char path[128] = {0};
    int cnt = 0;

    if ((dirp = opendir("/proc/")) == NULL) return -1;

    while ((dir = readdir(dirp)) != NULL)
    {
        if (dir->d_name && (atoi(dir->d_name)) > 0)
        {
            sprintf(path, "/proc/%s/status", dir->d_name);
            
            if ((fp = fopen(path, "r")) == NULL) continue;
            if (fscanf(fp, "Name:%s", name) == -1) 
                name[0] = '\0';
            if (fp != NULL) fclose(fp);

            if (strcmp(name, proc_name)) continue;
            cnt++;
            if (cnt > 1) {
                return 0;
            }
        }
    }

    if (dirp != NULL) closedir(dirp);

    return 1;
}

/**
 * @brief get proc exe path by pid
 *
 * @param pid  [in]  process id
 * @param path [out] exe path of process
 * @param size [in] size of buffer
 *
 * @return 0, if succ; -1, if failed
 */
int get_exec_path(pid_t pid, char *path, int size)
{
    char exe_path[64] = {0};
    char buf[256] = {0};

    if (pid <=0 || path == NULL) return -1;

    sprintf(exe_path, "/proc/%d/exe", pid);
    if (readlink(exe_path, buf, sizeof(buf)) <= 0) return -1;
    strncpy(path, buf, size);

    return 0;
}

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
int get_file_path(int fd, int pid, char *path, int size)
{
    char fd_path[128] = {0};
    char buf[256] = {0};

    if (fd < 0 || pid <= 0 || path == NULL || size <= 0) 
        return -1;  

    sprintf(fd_path, "/proc/%d/fd/%d", pid, fd);
    if (readlink(fd_path, buf, sizeof(buf)) <= 0) return -1;
    strncpy(path, buf, size);

    return 0;
}

/**
 * @brief get cmd exe line
 *
 * @param pid     [in]  pid
 * @param cmdline [out] cmd line
 * @param size    [in]  size of buffer
 *
 * @return 0, if succ; -1, failed.
 */
int get_cmdline(int pid, char *cmdline, int size)
{
    char buf[256] = {0};
    char path[128] = {0};
    int rt = -1, fd = -1, i = 0;

    if (cmdline == NULL || pid <=0 ) goto free;

    sprintf(path, "/proc/%d/cmdline", pid);
    if (access(path, F_OK | R_OK)) return -1;
    if ((fd = open(path, O_RDONLY)) < 0) return -1;
    if ((rt = read(fd, buf, sizeof(buf))) <= 0) goto free;

    while (i < rt) {
        if (buf[i] >= 127 || buf[i] < 32)
            cmdline[i] = ' ';
        else cmdline[i] = buf[i];
        if (i >= size) break;
        i++;
    }
    cmdline[i] = '\0';

free:
    if (fd > 0) close(fd);
    fd = -1;

    return i - 1;
}

/**
 * @brief SYSTEM 
 *
 * @param format
 * @param ...
 *
 * @return id of process, if succ; -1, if failed
 */
int SYSTEM(const char *format, ...)
{
    pid_t pid = -1;
    char buf[1024] = {0};
    va_list arg;

    va_start(arg, format);
    vsnprintf(buf, sizeof(buf), format, arg);
    va_end(arg);

    pid = cmd_exec(buf, NULL);
    usleep(10);

    return pid;
}

/**
 * @brief read system uptime
 *
 * @return uptime
 */
int read_uptime(void) {
    FILE *fp = NULL;
    int uptime = 0;

    fp = fopen("/proc/uptime", "r");
    if (fscanf(fp, "%u.", &uptime)) {}
    fclose(fp);

    return uptime;
}

/**
 * @brief create_state_file_by_name 
 *
 * @param app_name [in] process's name
 *
 * @return 0, if succ; -1, if failed
 */
int create_state_file_by_name(const char *app_name) {
    pid_t pid = 0;
    FILE *fp = NULL;
    char path[128] = {0};

    if (app_name == NULL) {
        return -1;
    }

    pid = getpid();
    snprintf(path, sizeof(path), "%s%s", APP_STATE_FILE_PATH, app_name);
    if ((fp = fopen(path, "w")) != NULL) {
        fprintf(fp, "pid:%d-uptime:%d-status:%d", pid, read_uptime(), APP_STARTING);
    } else {
        perror("Open app state file failed");
    }
    fclose(fp);

    return 0;
}

/**
 * @brief unlink_state_file_by_name 
 *
 * @param app_name [in] process's name
 */
void unlink_state_file_by_name(const char *app_name) {
    char path[256] = {0};

    if (app_name == NULL) {
        return;
    }

    snprintf(path, sizeof(path), "%s%s", APP_STATE_FILE_PATH, app_name);
    if (access(path, F_OK) != 0) {
        return;
    }

    unlink(path);
}

/**
 * @brief is_state_file_exist 
 *
 * @param app_name [in] app_name
 *
 * @return 0, if state file exist; -1, if not exist
 */
int is_state_file_exist(const char *app_name) {
    char path[256] = {0};

    if (app_name == NULL) {
        return -1;
    }

    snprintf(path, sizeof(path), "%s%s", APP_STATE_FILE_PATH, app_name);
    if (access(path, F_OK) != 0) {
        return -1;
    }
    
    return 0;
}

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
int check_app_start_conflict(const char *app_name, const char *state_file, int (*app_stop_cb)(void)) {
    char app_state[128] = {0};
    char pid[10] = {0};
    char state[10] ={0};
    FILE *fp = NULL;

    if (is_state_file_exist(app_name) != 0) {
        return 0;
    }

    if ((fp = fopen(state_file, "r")) == NULL) {
        return errno;
    }
    if (fgets(app_state, sizeof(app_state), fp)) {}
    sscanf(app_state, "%*[^:]:%[^-]-%*[^:]:%*[^:]:%[0-9]", pid, state);
    fclose(fp);

    switch (atoi(state)) {
        case APP_STARTING:
        case APP_RUNNING:
            return 1;
        case APP_CRASHED:
        case APP_TIMEOUT:
            if (app_stop_cb) {
                app_stop_cb();
            }
            break;
        default:
            break;
    }

    kill(atoi(pid), SIGTERM);

    return 0;
}

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
int app_state_check(const char *app_name, int pid_num, int uptime, const char *state_file, int (*app_state_check_cb)(void)) {
    int now = 0;
    char *idx = NULL;
    int status = 0;
    char app_uptime[20] = {0};
    char app_state[128] = {0};
    FILE *fp = NULL;
    int chk_result = 0;
    int ret = -1;

    if (is_state_file_exist(app_name) != 0) {
        return APP_STOPPED;
    }

    now = read_uptime();
    fp = fopen(state_file, "r+");
    if (fgets(app_state, sizeof(app_state), fp)) {}
    idx = rindex(app_state, ':');
    status = atoi(++idx);
    fseek(fp, -strlen(idx), SEEK_CUR);
    sscanf(app_state, "%*[^:]:%*[^:]:%[^-]", app_uptime);

    if (pid_num) {
        if (!check_proc_unique(app_name)) {
            ret = APP_RUNNING;
        } else {
            ret = APP_STOPPED;
        }
    } else {
        if (app_state_check_cb != NULL) {
            ret = app_state_check_cb();
        }
    }

    switch (ret) {
        case APP_STARTING:
            if ((now - atoi(app_uptime)) >= uptime) {
                chk_result = APP_TIMEOUT;
            } else {
                chk_result = APP_STARTING;
            }
            fprintf(fp, "%d", chk_result);
            break;
        case APP_TIMEOUT:
            chk_result = APP_TIMEOUT;
            fprintf(fp, "%d", chk_result);
            break;
        case APP_RUNNING:
            chk_result = APP_RUNNING;
            fprintf(fp, "%d", chk_result);
            break;
        case APP_CRASHED:
            switch (status) {
                case APP_CRASHED:
                case APP_RUNNING:
                    chk_result = APP_CRASHED;
                    break;
                case APP_STARTING:
                    if ((now - atoi(app_uptime)) >= uptime) {
                        chk_result = APP_TIMEOUT;
                        fprintf(fp, "%d", chk_result);
                    } else {
                        chk_result = APP_STARTING;
                    }
                    break;
                case APP_TIMEOUT:
                    chk_result = APP_TIMEOUT;
                    break;
                default:
                    chk_result = APP_STARTING;
                    break;
            }
            break;
        default:
            break;
    }
    fclose(fp);
    
    return chk_result;
}
