#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <tcp.h>
#include <sys/socket.h>

int main(int argc, char **argv)
{
    int ret = 0;
    char buf[64] = {0};
    tcp_t *tcp = tcp_create();

    if (argc > 1) {
        //ret = tcp->connect(tcp, AF_INET, "172.21.34.55", 5001);
        ret = tcp->connect(tcp, AF_INET, NULL, 5001);
        if (ret < 0) goto over; 

        ret = tcp->send(tcp, "hi", 2);
        if (ret > 0) printf("send count: %d\n", ret);
    } else {
        ret = tcp->listen(tcp, AF_INET, NULL, 5001);
        if (ret < 0) goto over; 
        ret = tcp->accept(tcp);
        if (ret < 0) goto over; 

        ret = tcp->recv(tcp, buf, sizeof(buf));
        if (ret > 0) printf("recv: %s\n", buf);
    }
over:
    tcp->destroy(tcp);
    return 0;
}
