/*
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <arpa/inet.h>

int main()
{
    int fd[2] = {-1};
    struct sockaddr_in addr = {0};
    int len = sizeof(struct sockaddr_in);
    char buf[128] = {0};

    fd[0] = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd[0] < 0) perror("socket");
    fd[1] = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd[1] < 0) perror("socket");

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(5001);

    if (bind(fd[0], (struct sockaddr *)&addr, (socklen_t)len) < 0)
        perror("bind");

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(5002);
    if(connect(fd[1], (struct sockaddr *)&addr, (socklen_t)len) < 0)
        perror("connect");

    while (recv(fd[0], buf, 128, 0) > 0)
    {
        printf("ser recv: %s\n", buf);
        if ((len = send(fd[1], buf, strlen(buf), 0)) > 0)
            printf("ser send count: %d\n", len);
    }

    return 0;
}*/

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <arpa/inet.h>

int main()
{
    int fd = {-1};
    struct sockaddr_in addr = {0};
    //struct sockaddr_in addr1 = {0};
    int len = sizeof(struct sockaddr_in);
    char buf[128] = {0};

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) perror("socket");

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(5001);

    if (bind(fd, (struct sockaddr *)&addr, (socklen_t)len) < 0)
        perror("bind");

    int rt = 0;
    while (recvfrom(fd, buf, 128, 0, (struct sockaddr *)&addr, (socklen_t *)&len))
    {
        printf("ser recv: %s\n", buf);
        if ((rt = sendto(fd, buf, strlen(buf), 0, (struct sockaddr *)&addr, (socklen_t)len)) > 0)
            printf("ser send count: %d\n", rt);
    }

    return 0;
}
