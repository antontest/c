#include "socket_base.h"
#include "socket_app.h"

#define ipc_path "../ipc"

void on_close(void *arg)
{
    printf("on_close\n");
    
    return;
}

void on_recv(void *arg)
{
    printf("on_recv\n");
    
    return;
}

int main(int argc, char *argv[])
{
    //int clifd = -1;
    int len = 0;
    char buf[128] = {0};
    struct socket_impl sck;

    if (inet_server_create(&sck, SOCK_STREAM, NULL, 5001) == -1)
        return -1;

    //clifd = socket_accept(sck.fd);
    //printf("accept succ\n");
    //printf("fd = %d\n", clifd);
    sleep(1);
    socket_event_add(&sck.evl, SOCKET_ON_CLOSE, on_close, NULL);
    socket_event_add(&sck.evl, SOCKET_ON_RECV, on_recv, NULL);
    while (sck.cli_fd[0] == -1) usleep(200);
    while ((len = socket_recv(sck.cli_fd[0], buf, sizeof(buf))))
    {
		if (len <= 0) break;
        printf("len: %d, info: %s\n", len, buf);

        socket_send(sck.cli_fd[0], "hi,cli!", sizeof("hi,cli!"));
        //sleep(1);
    }

    printf("over\n");
    return 0;
}
