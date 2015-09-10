#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>

/*********************************************************
 **************    Function Declaration    ***************
 *********************************************************/
int pipe_test()
{
    pid_t pid = -1;
    int fd[2];
    char buf[64] = {0};

    if (pipe(fd) < 0) 
    {
        fprintf(stderr, "create pipe failed.\n");
        return -1;
    }

    pid = fork();
    switch (pid)
    {
        case -1:
            fprintf(stderr, "fork failed.\n");
            break;
        case 0:
            close(fd[1]);
            if (read(fd[0], buf, sizeof(buf)) > 0)
                printf("recv: %s", buf);
            break;
        default:
            close(fd[0]);
            if (write(fd[1], "hello\n", sizeof("hello\n")) > 0)
                wait(NULL);
            break;
    }


    return 0;
}


/**
 * @brief create and open fifo, if not exist, then create
 *
 * @param path [in] path of fifo
 * @param mode [in] like O_RDONLY, O_WRONLY, O_RDWRONLY, O_NONBLOCK
 *
 * @return fd of fifo, if succ; -1, if failed.
 */
int fifo_init(const char *path, int mode)
{
    int fifo_fd = -1;

    if (access(path, F_OK) == -1) 
    { 
        if (mkfifo(path, 0777) < 0) 
            return -1;
    }

    if ((fifo_fd = open(path, mode)) < 0) return -1;

    return fifo_fd;
}


/**
 * @brief close fifo, meanwhile remove the fifo file
 *
 * @param fifo_fd [in] fd of fifo
 *
 * @return 0, if succ; -1, if failed
 */
int fifo_deinit(int fifo_fd)
{
    char path[128] = {0};
    char name[256] = {0};

    /**
     * get path of fifo file by fifo_fd
     */
    if (fifo_fd < 0) return 0;
    sprintf(path, "/proc/self/fd/%d", fifo_fd);
    if (readlink(path, name, sizeof(name)) <= 0) return -1;

    /**
     * remove fifo file
     */
    if (access(path, F_OK) == -1) return 0;
    unlink(name);

    /**
     * close fifo
     */
    if (fifo_fd > 0) close(fifo_fd);

    return 0;
}

void sig_act(int sig)
{
    switch (sig)
    {
        case SIGINT:
            printf("Ctl + C\n");
            exit(0);
            break;
        case SIGTERM:
            printf("TERM\n");
            exit(0);
            break;
        default:
            printf("no act\n");
            break;
    }
}

/*********************************************************
 ******************    Main Function    ******************
 *********************************************************/
int main(int agrc, char *agrv[])
{
    int rt = 0; /* return value of function main */

    signal(SIGINT, sig_act);
    signal(SIGTERM, sig_act);
    while (1);

    return rt;
}
