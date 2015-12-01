#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <listener.h>
#include <sys/types.h>
#include <sys/socket.h>

static void on_accept(int fd, void *arg)
{
    printf("on_accept\n");
}

static void on_close(int fd, void *arg)
{
    printf("on_close\n");
}

static void on_recv(int fd, void *arg)
{
    char buf[128] = {0};
    recv(fd, buf, sizeof(buf), 0);
    printf("recv: %s\n", buf);
}

static void on_connect(int fd, void *arg)
{
    send(fd, arg, strlen(arg), 0);
    usleep(100);
    send(fd, arg, strlen(arg), 0);
    usleep(100);
    send(fd, arg, strlen(arg), 0);
}

int main(int argc, char **argv)
{
    listener_t *s = create_listener();
    if (argc < 2) {
        s->set_cb(s, ON_ACCEPT, on_accept, NULL);
        s->set_cb(s, ON_RECV, on_recv, NULL);
        s->set_cb(s, ON_CLOSE, on_close, NULL);

        s->listen(s, AF_INET, SOCK_STREAM, "172.21.34.63", 5001);
        sleep(20);
    } else {
        s->set_cb(s, ON_CONNECT, on_connect, "hi");
        s->connect(s, AF_INET, SOCK_STREAM, "172.21.34.63", 5001);
    }
    s->destroy(s);

    return 0;
}
