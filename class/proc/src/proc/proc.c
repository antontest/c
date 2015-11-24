#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <stdarg.h>
#include <fcntl.h>
#include <utils/utils.h>
#include <utils/enum.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "proc.h"

typedef enum ipc_type_t ipc_type_t;
enum ipc_type_t {
    IPC_PIPE = 1,
    IPC_FIFO,
    IPC_SHM
};

typedef struct private_ipc_t private_ipc_t;
struct private_ipc_t {
    /**
     * @brief public interface
     */
    ipc_t public;
    
    /**
     * @brief type of IPC
     */
    ipc_type_t type;

    /**
     * @brief handles
     */
    union {
        int pipe_fds[2];
#define pipe_fd   ipc.pipe_fds
#define pread_fd  ipc.pipe_fds[0]
#define pwrite_fd ipc.pipe_fds[1]
        
        /**
         * fifo
         */
        struct {
            /**
             * handle
             */
            int fd;

            /**
             * @brief open mode
             */
            int mode;

            /**
             * @brief fifo path
             */
            char *fifo_path;
        } fifo;
#define fifo_fd   ipc.fifo.fd
#define fifo_mode ipc.fifo.mode
#define fifo_path ipc.fifo.fifo_path

        /**
         * shared memory
         */
        struct {
            /**
             * handle
             */
            int id;

           /**
            * shared data
            */
            int created;

            /**
             * address of shared memory
             */
            struct shm_comm_t *addr;
        } shm;
#define shm_created    ipc.shm.created
#define shm_id         ipc.shm.id 
#define shm_addr       ipc.shm.addr
#define shm_state      ipc.shm.addr->state
#define shm_perm       ipc.shm.addr->permission
#define shm_size       ipc.shm.addr->shm_size
#define shm_data_size  ipc.shm.addr->data_size
#define shm_data       ipc.shm.addr->data 

        /**
         * @brief signal communication
         */
        struct {
            /**
             * @brief sigaction struct info
             */
            struct sigaction act;

            /**
             * @brief deal with signal
             */
            void (*signal_handler) (int sig,siginfo_t *info, void *text);
        } sig;
#define sig_act ipc.sig.act
    } ipc;

    /**
     * @brief read buffer
     */
    char *read_buffer;

    /**
     * @brief write buffer
     */
    char *write_buffer;
};
#define DEFAULT_READ_BUFFER_SIZE  (1024)
#define DEFAULT_WRITE_BUFFER_SIZE (1024)

METHOD(ipc_t, mkpipe_, int, private_ipc_t *this)
{
    if (pipe(this->pipe_fd)) return -1;

    return 0;
}

METHOD(ipc_t, mkfifo_, int, private_ipc_t *this, const char *pathname, int mode)
{
    char *p = NULL;

    this->type = IPC_FIFO;
    this->fifo_mode = mode;
    if (!pathname || this->fifo_path != NULL || mode < 0) return -1;

    this->fifo_path = strdup(pathname);
    if (!this->fifo_path) return -1;

    if (strstr(this->fifo_path, "/") != NULL) {
        p = this->fifo_path + strlen(this->fifo_path) - 1;
        while (p >= this->fifo_path) {
            if (*p == '/') break;
            p--;
        }
        *p = '\0';
        if (access(this->fifo_path, F_OK)) return -1;
    }

    if (!(mode & 0x01) && access(pathname, F_OK)) {
        if (mkfifo(pathname, 0777)) return -1;
    } 

    this->fifo_fd = open(pathname, mode);
    if (this->fifo_fd < 0) unlink(pathname);
    *p = '/';
    return this->fifo_fd;
}

METHOD(ipc_t, reopen_, int, private_ipc_t *this)
{
    char *p = NULL;

    this->type = IPC_FIFO;
    if (!this->fifo_path || this->fifo_mode < 0) return -1;

    if (strstr(this->fifo_path, "/") != NULL) {
        p = this->fifo_path + strlen(this->fifo_path) - 1;
        while (p >= this->fifo_path) {
            if (*p == '/') break;
            p--;
        }
        *p = '\0';
    }

    if (!(this->fifo_mode& 0x01) && access(this->fifo_path, F_OK)) {
        if (mkfifo(this->fifo_path, 0777)) return -1;
    } 

    this->fifo_fd = open(this->fifo_path, this->fifo_mode);
    if (this->fifo_fd < 0) unlink(this->fifo_path);
    *p = '/';
    return this->fifo_fd;
}

METHOD(ipc_t, mkshm, int, private_ipc_t *this, key_t key, size_t size)
{
    void *shmaddr = NULL;

    this->type = IPC_SHM;
    if (this->shm_created) return 0;

    this->shm_id = shmget(key, size + sizeof(struct shm_comm_t), IPC_CREAT|0666);
    if (this->shm_id < 0) return -1;

    shmaddr = shmat(this->shm_id, (void *)0, 0);
    if (shmaddr == (void *)-1) return -1;

    this->shm_addr    = (struct shm_comm_t *)shmaddr;
    this->shm_state   = SHM_CREATED;
    this->shm_size    = size;
    this->shm_created = 1;

    return 0;
}

METHOD(ipc_t, mksig_, void, private_ipc_t *this, void (*signal_handler) (int sig,siginfo_t *info, void *text))
{
    this->sig_act.sa_sigaction = (void *)signal_handler;
    this->sig_act.sa_flags = SA_SIGINFO;
}

METHOD(ipc_t, sigact_, int, private_ipc_t *this, int sig)
{
    return sigaction(sig, &this->sig_act, NULL);
}

METHOD(ipc_t, read_, int, private_ipc_t *this, void *buf, int size)
{
    int readed_size = 0;

    switch (this->type) {
        case IPC_FIFO:
            return read(this->fifo_fd, buf, size);
        case IPC_SHM:
            if (this->shm_state == SHM_CREATING || this->shm_state == SHM_EMPTY) return -1;
            while (this->shm_state != SHM_WRITED) usleep(100);
            readed_size = size < this->shm_data_size ? size : this->shm_data_size;
            memcpy(buf, this->shm_data, readed_size);
            this->shm_state = SHM_READED;
            return readed_size;
        default:
            return -1;
    }
}

METHOD(ipc_t, write_, int, private_ipc_t *this, void *buf, int size)
{
    int writed_size = 0;

    switch (this->type) {
        case IPC_FIFO:
            return write(this->fifo_fd, buf, size);
        case IPC_SHM:
            if (this->shm_state == SHM_CREATING || this->shm_state == SHM_EMPTY) return -1;
            while (this->shm_state == SHM_WRITED && this->shm_state != SHM_READED) usleep(100);
            
            this->shm_state     = SHM_WRITING;
            writed_size = size < this->shm_size ? size : this->shm_size;
            memcpy(this->shm_data, buf, writed_size);
            this->shm_state     = SHM_WRITED;
            this->shm_data_size = writed_size;
            return writed_size;   
        default:
            return -1;
    }
}

METHOD(ipc_t, close_, void, private_ipc_t *this)
{
    switch (this->type) {
        case IPC_FIFO:
            if (this->fifo_fd > 0) {
                close(this->fifo_fd);
                this->fifo_fd   = -1;
            
                if (this->fifo_path) {
                    unlink(this->fifo_path);
                }
            }

            break;
        case IPC_SHM:
            shmdt(this->shm_addr);
            shmctl(this->shm_id, IPC_RMID, 0);
            break;
        default:
            break;
    }

}

METHOD(ipc_t, get_fifo_fd, int, private_ipc_t *this)
{
    return this->fifo_fd;
}

METHOD(ipc_t, get_shm_addr, void *, private_ipc_t *this)
{
    return this->shm_addr;
}

METHOD(ipc_t, get_shm_size, int, private_ipc_t *this)
{
    return this->shm_size;
}

METHOD(ipc_t, get_shm_state, shm_state_t, private_ipc_t *this)
{
    return this->shm_state;
}

METHOD(ipc_t, get_shm_data, void *, private_ipc_t *this)
{
    return this->shm_data;
}

METHOD(ipc_t, destroy_, void, private_ipc_t *this)
{
    if (this->read_buffer != NULL) free(this->read_buffer);

    switch (this->type) {
        case IPC_FIFO:
            if (this->fifo_fd > 0) close(this->fifo_fd);
            this->fifo_fd   = -1;

            if (this->fifo_path) {
                unlink(this->fifo_path);
                free(this->fifo_path);
                this->fifo_path = NULL;
            }
            break;
        case IPC_SHM:
            shmdt(this->shm_addr);
            //shmctl(this->shm_id, IPC_RMID, 0);
            break;
        default:
            break;
    }
    free(this);
}

ipc_t *create_ipc()
{
    private_ipc_t *this;

    INIT(this,
        .public = {
            .mkpipe = _mkpipe_,
            .mkfifo = _mkfifo_,
            .reopen = _reopen_,
            .mkshm  = _mkshm,
            .mksig  = _mksig_,

            .sigact  = _sigact_,
            .read    = _read_,
            .write   = _write_,
            .close   = _close_,
            .destroy = _destroy_,

            .get_fifo_fd   = _get_fifo_fd,
            .get_shm_addr  = _get_shm_addr,
            .get_shm_size  = _get_shm_size,
            .get_shm_state = _get_shm_state,
            .get_shm_data  = _get_shm_data,
        },
        .fifo_path = NULL,
    );
    memset(&this->ipc, 0, sizeof(this->ipc));
    this->shm_addr = NULL;

    return &this->public;
}

#define APP_STATE_FILE_PATH "/var/run/app_state"

static char app_state_file_path[128] = APP_STATE_FILE_PATH;

/**
 * @brief set_app_state_file_path 
 */
void set_app_state_file_path(const char *path)
{
    strncpy(app_state_file_path, path, sizeof(app_state_file_path));
}

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
        if (rt == 127) rt = 0;
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
 * @brief process running count
 *
 * @param name [in] process name
 *
 * @return count of already running, if process unique;  
 *          otherwise return 0
 */
int proc_running_cnt(const char *proc_name)
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
        }
    }

    if (dirp != NULL) closedir(dirp);

    return cnt;
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
    int ret = -1;
    char buf[1024] = {0};
    va_list arg;

    va_start(arg, format);
    vsnprintf(buf, sizeof(buf), format, arg);
    va_end(arg);
    ret = cmd_exec(buf, NULL);

    return ret;
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
 * @brief read systime 
 */
long read_systime()
{
    struct sysinfo sys = {0};

    sysinfo(&sys);
    return sys.uptime;
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
    snprintf(path, sizeof(path), "%s%s", app_state_file_path, app_name);
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

    snprintf(path, sizeof(path), "%s%s", app_state_file_path, app_name);
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

    snprintf(path, sizeof(path), "%s%s", app_state_file_path, app_name);
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
    int app_conf_status = -1;

    if (is_state_file_exist(app_name) != 0) {
        return APP_STOPPED;
    }

    now = read_uptime();
    fp = fopen(state_file, "r+");
    if (fgets(app_state, sizeof(app_state), fp)) {}
    idx = rindex(app_state, ':');
    status = atoi(++idx);
    fseek(fp, -strlen(idx), SEEK_CUR);
    sscanf(app_state, "%*[^:]:%*[^:]:%[^-]-%*[^:]:%d", app_uptime, &app_conf_status);

    if (pid_num) {
        if (proc_running_cnt(app_name) >= 1) {
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
        case APP_STOPPED:
            if (app_conf_status == APP_STARTING) chk_result = APP_TIMEOUT;
            else chk_result = APP_CRASHED;
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

ENUM(app_state_str, APP_NOAPP, APP_TIMEOUT, 
    "no app",
    "running",
    "crashed",
    "stopped",
    "starting",
    "timeout"
);
/**
 * @brief print app state 
 *
 * @param app   app name
 * @param state app state
 */
void print_app_state(const char *app, app_state_t state)
{
    if (!app) return;
    printf("[%s] %s\n", app, enum_to_name(app_state_str, state));
}


/**
 * @brief app_state_string 
 */
const char *app_state_string(app_state_t state)
{
    return enum_to_name(app_state_str, state);
}

