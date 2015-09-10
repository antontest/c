#include "proc.h"

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
 * @param name [in] process name
 *
 * @return pid , if succ; -1, if failed.
 */
int get_pid_by_name(const char *proc_name)
{
    struct dirent *dir = NULL;
    DIR *dirp = NULL;
    pid_t pid = -1;
    FILE *fp = NULL;
    char name[64] = {0};
    char path[128] = {0};

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
            pid = atoi(dir->d_name);
            break;
        }
    }

    if (dirp != NULL) closedir(dirp);

    return pid;
}

/**
 * @brief get process id by process name
 *
 * @param name [in] process name
 *
 * @return pid , if succ; -1, if failed.
 */
int get_id_by_name(const char *name)
{
    FILE *fp = NULL;
    char cmd[128] = {0};
    char ret_str[1024] = {0};
    if (name == NULL) return -1;

    sprintf(cmd, "ps -a | grep %s | \
            grep -v 'grep' | awk '{ print $1 }'", name);
    if ((fp = popen(cmd, "r")) == NULL) return -1;

    if (fgets(ret_str, sizeof(ret_str), fp)) ;
    pclose(fp);

    return atoi(ret_str);
}

/**
 * @brief get process name by pid
 *
 * @param pid        [in]  process id
 * @param proc_name  [out] process name
 * @param size       [in]  buffer size of proc_name
 *
 * @return 0, if succ; -1, if failed.
 */
int get_proc_name_by_pid(pid_t pid, char *proc_name, int size)
{
    char path[64] = {0};
    char buf[128] = {0};
    char *p = buf;
    int len = 0;

    if (pid < 0) return -1;
    sprintf(path, "/proc/%d/exe", pid);
    if ((len = readlink(path, buf, sizeof(buf))) < 0)
        return -1;

    while (len > 0 && buf[len -1] != '/') len--;
    p += len;
    strncpy(proc_name, p, size);

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
int get_name_by_pid(pid_t pid, char *name, int size)
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

const char* get_proc_name(const char *name)
{
    while (*name != '\0' && ( *name == '.' || *name == '/' ))
        name++;

    return name;
}

/**
 * @brief check_proc_unique 
 *
 * @param name [in] process name
 *
 * @return 1, if process unique; otherwise return 0
 */
int check_proc_unique(const char *name)
{
    FILE *fp = NULL;
    char cmd[256] = {0};
    char buf[518] = {0};
    int task_cnt = 0;

    name = get_proc_name(name);
    sprintf(cmd, "ps -a | grep %s | grep -v grep | \
            awk '{ print $1}'", name);
    if ((fp = popen(cmd, "r")) == NULL) return -1;
    while (fgets(buf, sizeof(buf), fp) != NULL)
    {
        if (strlen(buf) && atoi(buf) > 0)
            task_cnt++;
        if (task_cnt > 1) break;
    }

    if (fp != NULL) pclose(fp);
    if (task_cnt > 1) return 0;
    return 1;
}

/**
 * @brief check process is unique 
 *
 * @param name [in] process name
 *
 * @return pid of already running, if process unique; \ 
 *          otherwise return 0
 */
int is_proc_unique(const char *proc_name)
{
    struct dirent *dir = NULL;
    DIR *dirp = NULL;
    pid_t pid0 = get_pid_by_name(proc_name);
    pid_t pid1 = -1;
    FILE *fp = NULL;
    char name[64] = {0};
    char path[128] = {0};

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
            pid1 = atoi(dir->d_name);
            if (pid0 != pid1)
                break;
        }
    }

    if (dirp != NULL) closedir(dirp);
    if (pid0 != pid1) return pid1;

    return 0;
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
int get_exe_path_by_pid(pid_t pid, char *path, int size)
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

/*
static int visible(unsigned int c, char *buf)
{
    int rt = 0;

    if (c == '\t') goto raw;

    if (c < 32 || c >= 127) {
        c = ' ';
        rt += 1;
    }

    if (c >= 128) {
        c -= 128;
        *buf++ = 'M';
        *buf++ = '-';
        rt += 2;
    }

    // 0-31 ASCII control character
    if (c < 32 || c == 127) {
        *buf++ = '^';
        c ^= 0x40;
        c = ' ';
        rt += 1;
    }

raw:
    *buf++ = c;
    *buf++ = '\0';

    return rt + 1;
}
*/

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
