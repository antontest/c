#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <local.h>

int main(int argc, char **argv)
{
    int ret = 0;
    char buf[128] = {0};
    struct sockaddr *addr = NULL;
    local_socket_t *sck = create_local_socket();

    sck->socket(sck, SOCK_DGRAM);
    addr = sck->init_addr(sck, "./test_local_socket");
    //sck->init_addr(sck, "./test_local_socket");
    if (argc > 1) {
        sck->bind(sck);
        sck->listen(sck, 5);
        sck->accept(sck, NULL);
        //ret = sck->recv(sck, buf, sizeof(buf), 0);
        ret = sck->recvfrom(sck, buf, sizeof(buf), 0, NULL);
        printf("recv: %s, ret: %d\n", buf, ret);
    } else {
        //sck->connect(sck);
        ret = sck->sendto(sck, "hi", 3, 0, addr); 
        //ret = sck->send(sck, "hi", 3, 0); 
        printf("ret: %d\n", ret);
        perror("send()");
    }

    printf("over\n");
    sck->destroy(sck);
    return 0;
}
