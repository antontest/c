/**
 * @file cmd_agent.c
 * @brief cmd agent
 * @author Antonio_an
 * @version v1.0
 * @date 2015-05-30
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <sys/sysinfo.h>
#include <sys/errno.h>
#include <sys/wait.h>
#include <sys/select.h>

#define CMD_AGENT_ERROR(...) \
    do { \
        fprintf(stderr, "\033[1;35m[ Function %s ] [ line %d ] \033[0m", \
                __func__, __LINE__); \
        fprintf(stderr, ##__VA_ARGS__); \
        fprintf(stderr, "\n"); \
    } while(0);

#define DFL_TASK_Q_NUM 1
#define DFL_RECENT_TASK_NUM 5
#define DFL_TASK_TIME_OUT 15
#define DFL_SELECT_TIME_OUT 5

#define FIFO_DIR_PATH "./tmp/run/"

typedef struct task_entry task_entry_t;
typedef struct task_q_entry task_q_entry_t;

/**
 * @brief save parameter
 * 1. config file path
 * 2. task_q_num
 * 3. recent_task_num
 * 4. select_time_out
 */
struct conf {
    /**
     * config file's path
     */
    char *file_path;

    /**
     * total number of task_q 
     */
    int task_q_num;

    /**
     * total number of recent task 
     */
    int recent_task_num;

    /**
     * select timeout
     */
    int select_time_out;

} conf = {
    .file_path = "./cmd_agent.cfg",
    .task_q_num = DFL_TASK_Q_NUM,
    .recent_task_num = DFL_RECENT_TASK_NUM,
    .select_time_out = DFL_SELECT_TIME_OUT
};

/**
 * @brief task
 * 1. task_t -- process id of task
 * 2. cmd
 * 3. iskilled
 * 4. create_time
 * 5. start_exec_time
 * 6. finish_time
 * 7. next_task
 */
struct task_entry {
    /**
     * process id of task
     */
    long task_t;

    /**
     * task command
     */
    char *cmd;

    /**
     * is been killed
     */
    bool iskilled;

    /**
     * task creation time
     */
    long create_time;

    /**
     * task start exec time
     */
    long start_exec_time;

    /**
     * task finish time
     */
    long finish_time;

    /**
     * pointer of next task
     */
    task_entry_t *next_task;
};

/**
 * @brief task queue
 * 1. total_task_num
 * 2. task_num
 * 3. task_q_time_out
 * 4. fifo_fd
 * 5. fifo_path
 * 6. sem_q
 * 7. stop_dequeue
 * 8. pthr_t
 * 9. mut
 * 10. curr_task
 * 11. first_task
 * 12. last_task
 */
struct task_q_entry { 
    /**
     * total task num in the queue
     */
    long total_task_num;

    /**
     * the numbers of task in the queue
     */
    long task_num;

    /**
     * task time out is this queue
     */
    int task_q_time_out;

    /**
     * fifo fd
     */
    int fifo_fd;

    /**
     * fifo file's path
     */
    char *fifo_path;

    /**
     * semapthore
     */
    sem_t sem_q;

    /**
     * do stop queue , use to stop thread
     */
    bool stop_dequeue;

    /**
     * pthread id
     */
    pthread_t pthr_t;

    /**
     * mutex
     */
    pthread_mutex_t mut;

    /**
     * the currently executing task
     */
    task_entry_t *curr_task;

    /**
     * head of task queue
     */
    task_entry_t *first_task;

    /**
     * tail of task queue
     */
    task_entry_t *last_task;
} task_q_entry[4] = {0};

/**
 * \brief get name and value from a line splited by separator
 *
 * param char buf[] [in] line buf
 * param const char *separator [in] separator
 * param char **name     [out] name
 * param char **value    [out] value
 */
static void cfg_line_split(char buf[], const char *separator, char **name, char **value)
{
    char *p = NULL;
    int i = 0, len = 0;

    if (buf == NULL || separator == NULL || strlen(separator) < 1) {
        name = NULL;
        value = NULL;
        return ;
    }

    p = strtok(buf, separator);
    len = strlen(p);
    if (len < 1) {
        name = NULL;
        value = NULL;
        return ;
    }

    *name = buf;
    buf[len] = '\0';

    p = strtok(NULL, separator);
    if (strlen(p) < 1) {
        name = NULL;
        value = NULL;
        return ;
    }

    *value = buf + len + 1;
}

/**
 * @brief get_sys_uptime 
 *
 * @return uptime, if succ
 *             -1, if fail
 */
static long get_sys_uptime()
{
    struct sysinfo uptime = {0};

    if (!sysinfo(&uptime)) return uptime.uptime;
    return -1;
}

/**
 * @brief create_task 
 *
 * @param cmd[in] task command
 *
 * @return task_entry_t*, if succ
 *                  NULL, if fail
 */
static task_entry_t *create_task(const char* cmd)
{
    task_entry_t *p_task;
    char *p_cmd = NULL;
    if (NULL == cmd) return NULL;

    /**
     * create task and save its creation time
     */
    p_task = (task_entry_t*)malloc(sizeof(task_entry_t));
    if (NULL == p_task) {
        free(p_task);
    } else {
        memset(p_task, 0, sizeof(task_entry_t));
        p_cmd = strdup(cmd);

        if (!p_task || ((p_task->create_time = get_sys_uptime()) < 0)) {
            free(p_task);
            p_task = NULL;
        } else {
            p_task->cmd = p_cmd;
        }
    }

    return p_task;
}

/**
 * @brief destory_task 
 *
 * @param task
 *
 * @return 0, if succ
 *        -1, if fail
 */
static int destory_task(task_entry_t *task)
{
    if (!task) return 0;

    /**
     * free cmd firstly
     */
    if (NULL != task->cmd) {
        free(task->cmd);
        task->cmd = NULL;
    }

    /**
     * free task secondly
     */
    free(task);
    task = NULL;

    return 0;
}

/**
 * @brief exec_task 
 *
 * @param task
 *
 * @return 0, if succ
 *        -1, if fail
 */
static int exec_task(task_entry_t *task)
{
    pid_t pid = 0;
    int rt = 0;
    if (task == NULL || task->cmd == NULL) return -1;

    /**
     * execute shell command
     */
    if ((pid = fork()) < 0) {
        printf("fork failed.\n");
        return -1;
    } else if (pid == 0){
        setpgrp();
        if (execl("/bin/sh", "sh", "-c", task->cmd, (char *)0)) {
            CMD_AGENT_ERROR("execle error.");
            return -1;
        }
        _exit(127);
    } else {
        task->task_t = pid;
        task->start_exec_time = get_sys_uptime();
        
        while (waitpid(pid, NULL, 0) < 0) {
            if (errno != EINTR) return -1;
        }

    }

    return 0;
}

/**
 * @brief enqueue_task_q 
 *
 * @param *task_q
 * @param *task_t
 *
 * @return 0, if succ
 *        -1, if fail
 */
static int enqueue_task_q(task_q_entry_t *task_q, task_entry_t *task)
{
    if (task_q == NULL || task == NULL) return -1;

    pthread_mutex_lock(&task_q->mut);
    if (task_q->first_task == NULL) {
        task_q->first_task = task;
        task_q->last_task = task;
        task_q->last_task->next_task = NULL;
    } else {
        task_q->last_task  = task;
        task_q->last_task->next_task = task;
    }

    task_q->task_num++;
    sem_post(&task_q->sem_q);
    pthread_mutex_unlock(&task_q->mut);
    
    return 0;
}

/**
 * @brief dequeue_task_q 
 *
 * @param task_q
 *
 * @return task_entry_t*, if succ
 *                  NULL, if fail
 */
static task_entry_t* dequeue_task_q(task_q_entry_t *task_q)
{
    task_entry_t *task = NULL;

    sem_wait(&task_q->sem_q);

    pthread_mutex_lock(&task_q->mut);
    task = task_q->first_task;
    task_q->first_task = task->next_task;
    task_q->curr_task = task;
    task_q->task_num--;
    pthread_mutex_unlock(&task_q->mut);
    
    return task;
}

/**
 * @brief handle_task
 *
 * @param arg
 *
 * @return 
 */
static void* handle_task(void *arg)
{
    task_q_entry_t *task_q = (task_q_entry_t *)arg;
    task_entry_t *task = NULL;

    if (task_q == NULL) return NULL;

    while(!task_q->stop_dequeue) {
        if ((task = dequeue_task_q(task_q)) != NULL) {
            exec_task(task);
            destory_task(task);
            task_q->curr_task = NULL; 
        }
    }
}

/**
 * @brief init_task_q_entry 
 *
 * @param task_q
 *
 * @return 0, if succ
 *        -1, if fail
 */
static int init_task_q_entry()
{
    int i=0;

    for (i=0; i < conf.task_q_num; i++) {
        sem_init(&task_q_entry[i].sem_q, 0, 0);
        task_q_entry[i].stop_dequeue = false;

        pthread_mutex_init(&task_q_entry[i].mut, NULL);
        if (pthread_create(&task_q_entry[i].pthr_t, NULL, handle_task, &task_q_entry[i])) {
            CMD_AGENT_ERROR("Create thread failed.");
            return -1;
        }
    }

    return 0;
}

/**
 * @brief destory_task_q_entry 
 *
 * @param task_q
 *
 * @return 0, if succ
 */
static int destory_task_q_entry(task_q_entry_t task_q)
{
    task_entry_t *task = NULL;

    while (task_q.task_num-- > 0) {
        task = task_q.first_task;
        task_q.first_task = task->next_task;
        destory_task(task);
    }

    return 0;

}

/**
 * @brief poll_task 
 *
 * @param fds
 *
 * @return 0, if succ
 *        -1, if fail
 */
static int poll_task(fd_set fds)
{
    int i = 0, rt = 0;
    char cmd[1024] = {0};
    task_entry_t *task = NULL;

    for(i= 0; i < conf.task_q_num; i++) {
        if (FD_ISSET(task_q_entry[i].fifo_fd, &fds))
            break;
    }

    if ((rt = read(task_q_entry[i].fifo_fd, cmd, 1024)) == 0)
        return -1;

    cmd[rt] = '\n';
    if ((task = create_task(cmd)) == NULL) return -1;

    if (enqueue_task_q(&task_q_entry[i], task) < 0) {
        destory_task(task);
        task = NULL;
        return -1;
    }

    return 0;
}

static int get_args(int agrc, char *args[])
{
    return 0;
}


/**
 * @brief read_config_file 
 *
 * @return 0, if succ
 *        -1, if fail
 */
static int read_config_file()
{
    FILE* fp = NULL;
    char buf[1024] = {0};
    char *name = NULL, *value = NULL;
    int rt = 0, param_count = 0, i = 0;

    if (NULL == conf.file_path) {
        CMD_AGENT_ERROR("Config file's path is NULL.");
        return -1;
    }

    /**
     * \brief get info from cmd_agent.cfg
     *
     * 1. task_q_num
     * 2. recent_task_num
     * 3. select_time_out
     * 4. conf.task_q_time_out[]
     *
     */
    if ((fp = fopen(conf.file_path, "r")) == NULL) {
        CMD_AGENT_ERROR("Open config file failed.");
        return -1;
    }

    while (fgets(buf, sizeof(buf), fp) != NULL) {
        /**
         * get name and value from one line splited by "="
         */
        cfg_line_split(buf, "=", &name, &value);
        if (name == NULL || value == NULL) continue;

        /**
         * compare by name
         */
        if (!strncmp(name, "task_q_num", strlen(name))) {
            rt = atoi(value);        
            if(rt <= 0) rt = DFL_TASK_Q_NUM;
            conf.task_q_num = rt;
            param_count++;
        } else if (!strncmp(name, "recent_task_num", strlen(name))) {
            rt = atoi(value);        
            if(rt <= 0) rt = DFL_RECENT_TASK_NUM;
            conf.recent_task_num = rt;
            param_count++;
        }

        /**
         * get timeout from value by strtok
         */
        else if (!strncmp(name, "task_q_time_out", strlen(name))) {
            value = strtok(value, ",");

            for (i = 0; i < conf.task_q_num; i++) {
                if (value == NULL) {
                    task_q_entry[i].task_q_time_out = DFL_TASK_TIME_OUT;
                } else {
                    rt = atoi(value);
                    if (rt <= 0) {
                    task_q_entry[i].task_q_time_out = DFL_TASK_TIME_OUT;
                    } else { 
                        task_q_entry[i].task_q_time_out = rt;
                    }
                }
                param_count++;
            }
        } else if (!strncmp(name, "select_time_out", strlen(name))) {
            rt = atoi(value);        
            if(rt <= 0) rt = DFL_SELECT_TIME_OUT;
            conf.select_time_out = rt;
            param_count++;
        }
    }

    if (param_count != ( 3 + conf.task_q_num )) {
        CMD_AGENT_ERROR("The configation info is not complete.");
        fclose(fp);
        fp = NULL;
        return -1;
    }

    fclose(fp);
    fp = NULL;

    return 0;
}


/**
 * @brief create_multi_dir 
 *
 * @param dir_path
 *
 * @return 0, if succ
 *        -1, if fail
 */
static int create_multi_dir(const char *dir_path)
{
    int len = 0, i = 0;
    char *path = NULL;

    if(dir_path == NULL) return -1;
    if ((path = strdup(dir_path)) == NULL) return -1;

    for(i = 1; i < len; i++) {
        if (path[i] == '/') {
            path[i] = '\0';

            if (access(path, F_OK) < 0) {
                if (mkdir(path, 0777) < 0) return -1;
            }

            path[i] = '/';
        }
    }

    free(path);
    path = NULL;

    return 0;
}

/**
 * @brief init_and_open_fifo 
 *
 * @return 0, if succ
 *        -1, if fail
 */
static int init_and_open_fifo()
{
    int i = 0;
    char fifo_path[128] = {0};
    char *path = NULL;
    struct stat st = {0};
    int fd = 0;

    if (access(FIFO_DIR_PATH, F_OK) < 0) {
        if (create_multi_dir(FIFO_DIR_PATH) < 0) {
            CMD_AGENT_ERROR("mkdir FIFO_DIR_PATH error.");
            return -1;
        }
    }

    if (access(FIFO_DIR_PATH, R_OK | W_OK) < 0) {
        CMD_AGENT_ERROR("Have no read or write permission of dir FIFO_DIR_PATH.");
        return -1;
    }

    if(stat(FIFO_DIR_PATH, &st) < 0) {
        CMD_AGENT_ERROR("Can't get stat of dir FIFO_DIR_PATH.");
        return -1;
    }

    if (!S_ISDIR(st.st_mode)) {
        unlink(FIFO_DIR_PATH);
        if (create_multi_dir(FIFO_DIR_PATH) < 0) {
            CMD_AGENT_ERROR("mkdir FIFO_DIR_PATH error.");
            return -1;
        }
    }
    
    for (i = 0; i < conf.task_q_num; i++) {
        sprintf(fifo_path, "%s%d", FIFO_DIR_PATH, i);

        if (!access(fifo_path, F_OK))
            unlink(fifo_path);

        if (mkfifo(fifo_path, 0777) < 0) {
            CMD_AGENT_ERROR("Create FIFO failed.");
            return -1;
        }

        task_q_entry[i].fifo_path = strdup(fifo_path);
        if (task_q_entry[i].fifo_path == NULL) {
            free(task_q_entry[i].fifo_path);
            CMD_AGENT_ERROR("Apply for fifo path's memeroy failed.");
            return -1;
        }
        strcpy(task_q_entry[i].fifo_path, fifo_path);

        if (access(fifo_path, R_OK ) < 0) {
            CMD_AGENT_ERROR("Have no read permission.");
            return -1;
        }

        if ((task_q_entry[i].fifo_fd = open(fifo_path, O_RDONLY | O_NONBLOCK)) < 0) {
            CMD_AGENT_ERROR("Open fifo file failed.");
            return -1;
        }
    }   
}

/**
 * @brief uninit_and_close_fifo 
 *
 * @return 0, if succ
 *        -1, if fail
 */
static int uninit_and_close_fifo()
{
    int i = 0;

    for(i = 0; i < conf.task_q_num; i++) {
       close(task_q_entry[i].fifo_fd);
        unlink(task_q_entry[i].fifo_path);
        free(task_q_entry[i].fifo_path);
    }
}

/**
 * @brief process_over_handle 
 *
 * @return 
 */
static int process_over_handle()
{
    int i = 0;

    for(i = 0; i < conf.task_q_num; i++) {
        task_q_entry[i].stop_dequeue = true;
        destory_task_q_entry(task_q_entry[i]);
        sem_destroy(&task_q_entry[i].sem_q);
    }

    uninit_and_close_fifo();

    return -1;
}

/**
 * @brief kill_task 
 *
 * @return 
 */
static int kill_task()
{
    int i = 0;
    task_entry_t *curr_task = NULL;

    for (i = 0; i < conf.task_q_num; i++) {
        if ((curr_task = task_q_entry[i].curr_task) == NULL) continue;

        if (curr_task->iskilled) {
            kill(curr_task->task_t, SIGKILL);
            return 0;
        }

        if ((get_sys_uptime() - curr_task->start_exec_time) > task_q_entry[i].task_q_time_out) {
            kill(0 - curr_task->task_t, SIGTERM);
            curr_task->iskilled = true;
        }
    }

    return 0;
}

/**
 * @brief signal_hander 
 *
 * @param sig
 */
static void signal_handler(int sig)
{
    switch (sig) {
        case SIGINT:
        case SIGTERM:
        case SIGKILL:
        case SIGSTOP:
            process_over_handle();
            break;
        default:
            break;
    }

    exit(0);
}

/**
 * @brief main 
 *
 * @param agrc
 * @param argv[]
 *
 * @return 0, if success
 *        -1, if failed
 */
int main(int agrc, char *argv[])
{
    struct timeval tv = {0};
    fd_set fds = {0};
    int i = 0, rt = 0;

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGKILL, signal_handler);
    signal(SIGSTOP, signal_handler);

    if (read_config_file() == -1) {
        CMD_AGENT_ERROR("Read config file failed.");
        return -1;
    }

    if (init_and_open_fifo() < 0) {
        CMD_AGENT_ERROR("Fifo init and open error.");
        return -1;
    }

    if (init_task_q_entry() < 0) {
        CMD_AGENT_ERROR("Task queue init failed.");
        return -1;
    }

    while(1) {
        FD_ZERO(&fds);
        for(i = 0; i < conf.task_q_num; i++)
            FD_SET(task_q_entry[i].fifo_fd, &fds);

        tv.tv_sec = conf.select_time_out;
        tv.tv_usec = 0;
        rt = select(task_q_entry[conf.task_q_num - 1].fifo_fd + 10, \
                &fds, NULL, NULL, &tv);
       
        switch(rt) {
            case -1 :
                break;
            case 0 :
                kill_task();
                break;
            default:
                poll_task(fds);
                break;
        }
    }

    return 0;
}
