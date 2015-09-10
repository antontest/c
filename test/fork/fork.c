#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <pthread.h>
#include <linux/kernel.h>
#include <signal.h>

void* p_kill(void *tid)
{
        pid_t *pid = (pid_t *)tid;

        sleep(2);
        if (0 < *pid) {
                printf("-pid = %d.\n", 0- *pid);
                kill(0 - *pid, SIGTERM);
                printf("kill.\n");
        }
}

int exec_task(const char *pcmd)
{
        if (NULL == pcmd) return -1;

        pthread_t pthid;
        pid_t pid;
        pid = getpid();
        printf("father pid is %d.\n", pid);

        if ((pid = fork()) < 0) {
                printf("fork error.\n");
                return -1;
        } else if (pid == 0) {
                execl("/bin/sh", "sh", "-c", pcmd, (char *)0);
        } else {
                pthread_create(&pthid, NULL, p_kill, &pid);

                while (waitpid(pid, NULL, 0) < 0) {
                        if (EINTR != errno)
                                return -1;
                }

                printf("child pid is %d.\n", pid);
        }

        return 0;
}


int main(int agrc, char *agrv[])
{
        if (agrc != 2) {
                printf("Please input shell command.\n");
                return -1;
        }

        exec_task(agrv[1]);

        return 0;
}
