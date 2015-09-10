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
void on_accept(int fd, void *arg)
{
    printf("on accept\n");
}

void on_recv(int fd, void *arg)
{
    char buf[128] = {0};

    socket_recv(fd, buf, sizeof(buf));
    printf("tcp recv: %s\n", buf);
}

/*********************************************************
 ******************    Main Function    ******************
 *********************************************************/
int main(int agrc, char *agrv[])
{
    int rt = 0; /* return value of function main */
    struct socket_impl sck = {0};

    inet_client_connect(&sck, SOCK_STREAM, "127.0.0.1", 5001);
    socket_event_add(&sck.evl, SOCKET_ON_ACCEPT, on_accept, NULL);
    socket_event_add(&sck.evl, SOCKET_ON_RECV, on_recv, NULL);
    socket_send(sck.fd, "hello", 6);
    
    return rt;
}
