/**
 * usual head files
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
//#include <socket/socket_app.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/*
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/netinet/in.h>
#include <sys/types.h>
#include <arpa/net.h>
*/

/*********************************************************
 **************    Function Declaration    ***************
 *********************************************************/

/*********************************************************
 ******************    Main Function    ******************
 *********************************************************/
int addfd(int epfd, int fd, int oneshot)
{
    struct epoll_event evt = {0};

    evt.data.fd = fd;
    evt.events |= EPOLLIN | EPOLLRDHUP | EPOLLET;
    if (oneshot) evt.events |= EPOLLONESHOT;
    return epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &evt);
}

int reset_oneshot(int epfd, int fd)
{
    struct epoll_event evt = {0};

    evt.data.fd = fd;
    //evt.events |= EPOLLIN | EPOLLHUP | EPOLLRDHUP | EPOLLONESHOT;
    evt.events |= EPOLLIN | EPOLLRDHUP | EPOLLONESHOT;
    return epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &evt);
}

int main(int agrc, char *agrv[])
{
    int rt = 0; /* return value of function main */
    int fd = -1;
    int clifd = -1;
    int epfd = -1;
    int nready = 0;
    int i = 0;
    char buf[256] = {0};
    struct epoll_event events[10];
    struct sockaddr_in addr;

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return -1;
    addr.sin_port = htons(5001);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("0.0.0.0");
    if (bind(fd, (struct sockaddr *)&addr, sizeof(struct sockaddr)) < 0) return -1;
    if (listen(fd, 5) < 0) return -1;

    epfd = epoll_create(10);
    addfd(epfd, fd, 0);

    while (1) {
        nready = epoll_wait(epfd, events, 10, -1);
        if (nready < 0) {
            printf("epoll _wait error\n");
            return -1;
        }

        for (i = 0; i < nready; i++) 
        {
            if (events[i].data.fd == fd) {
                clifd = accept(fd, NULL, NULL);
                printf("on_accpet\n");
                addfd(epfd, clifd, 1);
            } else if (events[i].events & EPOLLRDHUP) {
                printf("closed\n");
                close(events[i].data.fd);
            } else if (events[i].events & EPOLLHUP) {
                printf("close 1111\n");
                close(events[i].data.fd);
            } else if (events[i].events & EPOLLIN) {
                if (recv(events[i].data.fd, buf, sizeof(buf), 0) <= 0)
                {
                    printf("closed2222\n");
                    close(events[i].data.fd);
                }
                else printf("recv: %s\n", buf);
                reset_oneshot(epfd, events[i].data.fd);
            }
        }
    }

    return rt;
}
