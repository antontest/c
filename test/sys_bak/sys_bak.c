#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <errno.h>

/*********************************************************
 **************    Function Declaration    ***************
 *********************************************************/
int cmd_exec(const char *cmd, int *proc_pid)
{
    pid_t pid = -1;
    int state = -1;

    if (cmd == NULL) return -1;
    printf("%s\n", getenv("PATH"));
    pid = fork();
    switch (pid) 
    {
        case -1:
            state = -1;
            break;
        case 0:
            setpgrp();
            execl("/bin/sh", "sh", "-c", cmd, (char *)0);
            break;
        default:
            *proc_pid = pid;
            printf("pid2: %d\n", pid);

            while (waitpid(pid, &state, 0) < 0)
            {
                if (errno != EINTR)
                    break;
            }
            break;
    }
    
    return state;
}

/*********************************************************
 ******************    Main Function    ******************
 *********************************************************/
int main(int agrc, char *agrv[])
{
    int rt = 0; /* return value of function main */
    pid_t pid = -1;
    if (agrv[1] != NULL)
    cmd_exec(agrv[1], &pid);

    printf("pid0 = %d\n", getpid());
    printf("pid1 = %d\n", pid);

    return rt;
}
