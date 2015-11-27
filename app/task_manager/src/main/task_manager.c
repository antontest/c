/**
 * usual head files
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <thread/thread.h>
#include <thread/mutex.h>
#include <thread/bsem.h>
#include <thread/pool.h>
#include <utils/utils.h>
#include <utils/get_args.h>
#include <data/linked_list.h>
#include <data/queue.h>
#include <socket/socket.h>
#include <socket/event.h>
#include <socket/local.h>
#include <socket/message.h>
#include <proc.h>
#include <task_manager.h>
#include <time.h>

#define MSG_BODY_BUFF_SIZE (512)
#define CMD_AGENT_PATH "/home/anton/var/run/cmd_agent/"
typedef struct task_manager_t task_manager_t;
struct task_manager_t {
    /**
     * @brief task dealing thread pool
     */
    pool_t   *task_pool;

    /**
     * @brief task event recving
     */
    event_t  *evt;

    /**
     * @brief network module
     */
    socket_t *net;

    /**
     * @brief stop flags
     */
    int stop;

    /**
     * @brief handle sigal error
     */
    ipc_t *sig;

    /**
     * @brief send commend to cmd_agent
     */
    ipc_t *cmd;

    /**
     * @brief message body buffer
     */
    char *msg_body;
} *task_manager = NULL;

/*********************************************************
 **************    Function Declaration    ***************
 *********************************************************/
/**
 * @brief handle task from net
 */
static void command_task_handler(char *cmd)
{
    task_manager->cmd->write(task_manager->cmd, cmd, strlen(cmd));
}

/**
 * @brief time sysnc
 */
static void timesys_task_handler(void *arg)
{
    time_t now;
    struct tm *timenow;
    int fd = *(int *)arg;

    time(&now);
    timenow = localtime(&now);
    send(fd, timenow, sizeof(struct tm), 0);
}

/**
 * @brief recv message
 */
static void message_handler(int fd, void *arg)
{
    int recv_cnt = 0;
    task_msg_t task_msg_head = {0};
    void *task = NULL;
    void *task_arg = NULL;
    int msg_fd = fd;
    
    recv_cnt = recv(fd, &task_msg_head, sizeof(task_msg_head), 0);
    if (recv_cnt < sizeof(task_msg_head)) return;
    
    if (task_msg_head.len > 0)
        recv(fd, task_manager->msg_body, task_msg_head.len, 0);
    switch (task_msg_head.type) {
        case TASK_TYPE_COMMAND:
            task = command_task_handler;
            task_arg = task_manager->msg_body;
            break;
        case TASK_TYPE_SYSN_TIME:
            task = timesys_task_handler;
            task_arg = (void *)&msg_fd;
            break;
        default:
            break;
    }
    task_manager->task_pool->addjob(task_manager->task_pool, task, task_arg); 
}

/**
 * @brief deal with socket when it closed
 */
static void net_close_handler(int fd, void *arg)
{
    task_manager->evt->delete(task_manager->evt, fd, EVENT_ON_ALL);
    close(fd);
}

/**
 * @brief deal with socket when it connected
 */
static void net_connect_handler(int fd, void *arg)
{
    int accept_fd = accept(fd, NULL, 0);
    if (accept_fd < 1) return;

    task_manager->evt->add(task_manager->evt, accept_fd, EVENT_ON_RECV, (void *)message_handler, NULL);
    task_manager->evt->add(task_manager->evt, accept_fd, EVENT_ON_CLOSE, net_close_handler, NULL);
}

/**
 * @brief task_manager_deinit and free memory
 */
static void task_manager_deinit()
{
    if (!task_manager) return;

    if (task_manager->net) task_manager->net->destroy(task_manager->net);
    if (task_manager->task_pool) task_manager->task_pool->destroy(task_manager->task_pool);
    if (task_manager->evt) task_manager->evt->destroy(task_manager->evt);
    if (task_manager->cmd) task_manager->cmd->destroy(task_manager->cmd);
    if (task_manager->sig) task_manager->sig->destroy(task_manager->sig);
    if (task_manager->msg_body) free(task_manager->msg_body);
    free(task_manager);
}

/**
 * @brief handle signal
 */
static void signal_error_handler(int sig, siginfo_t *info, void *text)
{
    switch (sig) {
        case SIGINT:
        case SIGTERM:
        case SIGKILL:
        case SIGSTOP:
            task_manager_deinit();
            exit(1);
            break;
    }
}

/**
 * @brief init task manager
 */
static int task_manager_init()
{
    int fd = 0;
    INIT(task_manager, 
        .evt       = create_event(EVENT_MODE_EPOLL, 1000),
        .net       = create_socket(),
        .task_pool = create_pool(5),
        .sig       = create_ipc(),
        .cmd       = create_ipc(),
        .stop      = 0,
        .msg_body  = (char *)malloc(MSG_BODY_BUFF_SIZE),
    );
    
    if (!task_manager || 
        !task_manager->net || 
        !task_manager->evt || 
        !task_manager->sig ||
        !task_manager->cmd ||
        !task_manager->task_pool || 
        !task_manager->msg_body) return -1;

    fd = task_manager->net->listen(task_manager->net, AF_INET, SOCK_STREAM, IPPROTO_TCP, "%any", 5001);
    if (fd <= 0) {
        perror("task_manager listen()");
        return -1;
    }

    if (task_manager->evt->add(task_manager->evt, fd, EVENT_ON_ACCEPT, net_connect_handler, NULL) < 0) {
        printf("event add faild\n");
        return -1;
    }

    if (!access(CMD_AGENT_PATH"0", F_OK)) {
        if (task_manager->cmd->mkfifo(task_manager->cmd, CMD_AGENT_PATH"0", O_NONBLOCK|O_WRONLY) < 0)
            printf("mkfifo failed\n");
    }

    task_manager->sig->mksig(task_manager->sig, signal_error_handler);
    task_manager->sig->sigact(task_manager->sig, SIGINT);
    task_manager->sig->sigact(task_manager->sig, SIGTERM);
    task_manager->sig->sigact(task_manager->sig, SIGKILL);
    task_manager->sig->sigact(task_manager->sig, SIGSTOP);

    return 0;
}

/*********************************************************
 ******************    Main Function    ******************
 *********************************************************/
int main(int agrc, char *agrv[])
{
    /* return value of function main */
    int rt = 0; 
    int help_flag = 0;
    int time = 0;
    struct options opts[] = {
        {"-h", "--help", 0, RET_INT, ADDR_ADDR(help_flag)},
        {"-t", "--time", 1, RET_INT, ADDR_ADDR(time)},
        {NULL, NULL}
    };
    struct usage usg[] = {
        {"-h, --help", "show usage"},
        {"-t, --time", "task manager run time. 0 stands forever."},
        {NULL, NULL}
    };

    get_args(agrc, agrv, opts);
    if (help_flag) {
        print_usage(usg);
        return 0;
    }

    task_manager_init();
    if (!time) while (1) sleep(2);
    else sleep(time);
    task_manager_deinit();

    return rt;
}
