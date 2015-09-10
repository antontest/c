/**
 * usual head files
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

int main(int argc, char *argv[])
{
    struct sockaddr addr;
    struct sockaddr *p;
    struct sockaddr_in addr_in;

    addr.sa_family = AF_INET;
    strcpy(addr.sa_data, "eth0");
    p = &addr;
    memcpy(&addr_in, &addr, sizeof(addr));
    printf("ip: %s\n", inet_ntoa(addr_in.sin_addr));


    return 0;
}
