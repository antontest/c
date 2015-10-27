/**
 * usual head files
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <host.h>
#include <socket.h>

/*********************************************************
 **************    Function Declaration    ***************
 *********************************************************/

/*********************************************************
 ******************    Main Function    ******************
 *********************************************************/
int main(int agrc, char *agrv[])
{
    int rt = 0; /* return value of function main */

    /*
    socket_t *tcp = socket_create();
    tcp->listen(tcp, AF_INET, SOCK_STREAM, IPPROTO_TCP, "172.21.34.28", 5001);
    int tcp_fd = tcp->accept(tcp);
    if (tcp_fd > 0) {
        char buff[128] = {0};
        recv(tcp_fd, buff, sizeof(buff), 0);
        printf("recv: %s\n", buff);
    }
    tcp->destroy(tcp);

    socket_t *udp = socket_create();
    int udp_fd = udp->listen(udp, AF_INET, SOCK_DGRAM, IPPROTO_UDP, "172.21.34.28", 5001);
    if (udp_fd > 0) {
        printf("udp\n");
        char buff[64] = {0};
        recvfrom(udp_fd, buff, sizeof(buff), 0, NULL, 0);
        printf("recv: %s\n", buff);
    }
    udp->destroy(udp);
    */

    /*
    socket_t *tcp = socket_create();
    int fd = tcp->connect(tcp, AF_INET, SOCK_STREAM, IPPROTO_TCP, "172.21.34.28", 5001);
    if (fd > 0) {
        send(fd, "hello", 6, 0);
    }
    tcp->destroy(tcp);
    */

    /*
    socket_t *udp = socket_create();
    udp->connect(udp, AF_INET, SOCK_DGRAM, IPPROTO_UDP, "172.21.34.28", 5001);
    udp->send(udp, "hello", 6);
    udp->destroy(udp);
    */

    socket_t *tcp = socket_create();
    char buff[64] = {0};
    tcp->listen(tcp, AF_INET, SOCK_STREAM, IPPROTO_TCP, "172.21.34.28", 5001);
    tcp->accept(tcp);
    tcp->receive(tcp, buff, sizeof(buff), 0);
    printf("recv: %s\n", buff);
    sleep(3);
    printf("state: %d\n", tcp->get_state(tcp));
    tcp->receive(tcp, buff, sizeof(buff), 0);
    printf("recv: %s\n", buff);

    /*
    socket_t *udp = socket_create();
    char buff[64] = {0};
    udp->listen(udp, AF_INET, SOCK_DGRAM, IPPROTO_UDP, "172.21.34.28", 5001);
    udp->receive(udp, buff, sizeof(buff), 0);
    printf("recv: %s\n", buff);
    sleep(5);
    udp->receive(udp, buff, sizeof(buff), 0);
    printf("recv: %s\n", buff);
    */

    return rt;
}
