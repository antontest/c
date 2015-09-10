/**
 * usual head files
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <socket/socket_app.h>

/*********************************************************
 **************    Function Declaration    ***************
 *********************************************************/

/*********************************************************
 ******************    Main Function    ******************
 *********************************************************/
/*
int main(int agrc, char *agrv[])
{
    int rt = 0;
    int fd = -1;
    int connfd = -1;
    int flag = 0;
    struct sockaddr_in addr;
    struct sctp_initmsg initmsg;
    struct sctp_sndrcvinfo info;
    struct sctp_event_subscribe event = {0};
    char buf[256] = {0};

    if ((fd = socket(PF_INET, SOCK_STREAM, IPPROTO_SCTP)) < 0)
    {
        printf("socket faile\n");
        return -1;
    }

    addr.sin_family = PF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(5001);

    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        printf("bind failed\n");
        return -1;
    }

    initmsg.sinit_num_ostreams = 5;
    initmsg.sinit_max_instreams = 5;
    initmsg.sinit_max_attempts = 4;
    if (setsockopt(fd, IPPROTO_SCTP, SCTP_INITMSG, &initmsg, sizeof(initmsg)) < 0)
    {
        printf("setsockopt sctp_initmsg failed\n");
        return -1;
    }

    event.sctp_data_io_event = 1;
    setsockopt(fd, IPPROTO_SCTP, SCTP_EVENTS, &event, sizeof(event));

    listen(fd, 5);

    //while (1)
    {
        connfd = accept(fd, (struct sockaddr *)NULL, (int *)NULL);
        printf("connect succ\n");
        if (rt = sctp_recvmsg(connfd, buf, sizeof(buf), \
                    (struct sockaddr *)NULL, 0, &info, &flag))
        {
            printf("stream: %d\n", info.sinfo_stream);
            printf("ret: %d\n", rt);
            printf("recv: %s\n", buf);
        }

    }

    return rt;
}*/

void on_conn(int fd, void *arg)
{
    printf("on_accept\n");

    return;
}

void on_recv(int fd, void *arg)
{
    int flag = 0;
    int rt = 0;
    char buf[128] = {0};
    struct sctp_sndrcvinfo info = {0};

    printf("on_recv\n");
    if (rt = sctp_recvmsg(fd, buf, sizeof(buf), \
                    (struct sockaddr *)NULL, 0, &info, &flag))
    {
        printf("stream: %d\n", info.sinfo_stream);
        printf("ret: %d\n", rt);
        printf("recv: %s\n", buf);
    }

    return;
}

int main(int argc, char *argv[])
{
    struct socket_impl sck = {0};

    sctp_client_connect(&sck, "127.0.0.1", 5001);
    socket_event_add(&sck.evl, SOCKET_ON_CONNECT, on_conn, NULL);
    socket_event_add(&sck.evl, SOCKET_ON_RECV, on_recv, NULL);
    sctp_sendmsg(sck.fd, "hello", 6, NULL, 0, 0, 0, 1, 0, 0);

    while (1);

    return 0;
}
