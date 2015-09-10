#include "socket_base.h"
#include "socket_app.h"

#define ipc_path "../ipc"

int main(int argc, char *argv[])
{
    int fd = -1;
    int clifd = -1;
    int len = 0;
    char buf[128] = {0};
    struct socket_impl sck;

    if ((fd = inet_server_create(&sck, SOCK_STREAM, NULL, 5001)) == -1)
        return -1;

    clifd = socket_accept(fd);
    printf("accept succ\n");
    while ((len = socket_recv(clifd, buf, sizeof(buf))))
    {
		if (len <= 0) break;
        printf("len: %d, info: %s\n", len, buf);

        socket_send(clifd, "hi", 3);
    }

    return 0;
}
