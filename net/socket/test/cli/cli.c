#include "socket_base.h"
#include "socket_app.h"

#define ipc_path "../ipc"

int main(int argc, char *argv[])
{
    int len = 0;
    char buf[128] = {0};
    struct socket_impl sck;

    if (inet_client_connect(&sck, SOCK_STREAM, "127.0.0.1", 5001) == -1)
        return -1;

    printf("connect succ\n");
    socket_send(sck.fd, "hi,ser!", sizeof("hi,ser!"));
    while ((len = socket_recv(sck.fd, buf, sizeof(buf))))
    //while (1)
    {
		if (len < 0) break;
        printf("len: %d, info: %s\n", len, buf);

        socket_send(sck.fd, "hi,ser!", sizeof("hi,ser!"));
        sleep(1);
    }

    printf("over\n");
    return 0;
}
