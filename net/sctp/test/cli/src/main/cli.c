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
void on_connect(int fd, void *arg)
{
    printf("on_connect\n");
}

/*********************************************************
 ******************    Main Function    ******************
 *********************************************************/
int main(int agrc, char *agrv[])
{
    int rt = 0; /* return value of function main */
    struct socket_impl sck = {0};

    inet_client_connect(&sck, SOCK_STREAM, "127.0.0.1", 5001);
    socket_event_add(&sck.evl, SOCKET_ON_CONNECT, on_connect, NULL);
    socket_send(sck.fd, "hello", 6);

    return rt;
}
