#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <event.h>
#include <socket_base.h>
event_t *evt;

void on_recv(int fd, void *arg)
{
    char buf[100] = {0};
    recv(fd, buf, sizeof(buf), 0);
    printf("recv: %s\n", buf);
}

void on_close(int fd, void *arg)
{
    printf("on_close\n");
    close(fd);

    evt->delete(evt, fd, EVENT_ON_ALL);
    //evt->delete(evt, fd, EVENT_ON_CLOSE);
    //evt->delete(evt, fd, EVENT_ON_RECV);
}

void on_accept(int fd, void *arg)
{
    int accept_fd = accept(fd, NULL, 0);
    evt->add(evt, accept_fd, EVENT_ON_RECV, on_recv, NULL);
    evt->add(evt, accept_fd, EVENT_ON_CLOSE, on_close, NULL);
}

void on_connect(int fd, void *arg)
{
    printf("on_connect\n");
}

int main(int argc, char **argv)
{
    //evt = create_event(EVENT_MODE_SELECT, 1000);
    evt = event_create(EVENT_MODE_EPOLL, 1000);
    socket_base_t *sck = create_socket_base();   
    int fd = 0;

    fd = sck->socket(sck, AF_INET, SOCK_STREAM, 0);
    if (fd < 1) return -1;
    //sck->bind(sck, "0.0.0.0", 5001);
    //sck->listen(sck, 5);
    //evt->add(evt, fd, EVENT_ON_ACCEPT, on_accept, NULL);
    
    evt->add(evt, fd, EVENT_ON_CONNECT, on_connect, NULL);
    if (sck->connect(sck, "172.21.34.63", 5001) == 0) {
        printf("connect succ\n");
    }
    sleep(2);
    evt->destroy(evt);
    return 0;
}
