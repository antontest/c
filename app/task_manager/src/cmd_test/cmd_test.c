#include <task_manager.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <socket/socket.h>
#include <socket/socket_common.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int main(int argc, char **argv)
{
    socket_t *sck = create_socket();
    task_msg_t *msg = NULL;

    if (!sck || argc < 2) return -1;
    if (sck->connect(sck, AF_INET, SOCK_STREAM, IPPROTO_TCP, "172.21.34.63", 5001) < 0) return -1;
    msg = (task_msg_t *)malloc(sizeof(task_msg_t) + 56);
    if (msg) {
        memset(msg, 0, sizeof(task_msg_t) + 56);
        msg->type = atoi(argv[1]);
        if (msg->type == TASK_TYPE_COMMAND) {
            msg->len  = 56;
            strcpy(msg->data, "mkdir /home/anton/t");
        }

        sck->send(sck, msg, sizeof(task_msg_t) + 56);
        if (msg->type == TASK_TYPE_SYSN_TIME) {
            int ret = 0;
            struct tm timenow;
            ret = sck->recv(sck, &timenow, sizeof(timenow), 1000);
            printf("ret: %d, local time: %s", ret, asctime(&timenow));
        }
    }
    free(msg);
    sck->destroy(sck);
    return 0;
}
