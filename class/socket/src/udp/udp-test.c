#include <udp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>

int main(int argc, char **argv)
{
    int ret = 0;
    char buf[64] = {0};
    udp_t *udp = udp_create();
    ret = udp->socket(udp, AF_INET, NULL, 5001);
    if (ret < 0) goto over;

    if (argc < 2) {
        /*
           ret = udp->connect(udp);
           if (ret < 0) goto over;
           ret = udp->send(udp, "hi", 2);
           */
        ret = udp->sendto(udp, "hi", 2, NULL, 0);
        if (ret > 0) printf("send count: %d\n", ret);
    } else {
        ret = udp->bind(udp);
        if (ret < 0) goto over;

        ret = udp->recvfrom(udp, buf, sizeof(buf), NULL, 0);
        if (ret > 0) printf("recv: %s\n", buf);
    }


over:
    udp->destroy(udp);
    return 0;
}
