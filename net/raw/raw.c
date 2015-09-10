#include <sys/types.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/types.h>
#include <netinet/in.h>
#include <netinet/udp.h>
#include <netinet/ip.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>
#include <arpa/inet.h>
#include <string.h>
#include <signal.h>
#include <net/if.h>
#include <stdio.h>
#include <sys/uio.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/filter.h>
#include <stdlib.h>
/*********************************************************
 *****************    Macro Defination    ****************
 *********************************************************/
#define DEBUG_ERROR(...) \
    do { \
        fprintf(stderr, "\033[1;35m[ Function %s ] [ line %d ] \033[0m", \
                __func__, __LINE__); \
        fprintf(stderr, ##__VA_ARGS__); \
        fprintf(stderr, "\n"); \
    } while(0);

/*********************************************************
 **************    Function Declaration    ***************
 *********************************************************/

/*********************************************************
 ******************    Main Function    ******************
 *********************************************************/
int main(int agrc, char *agrv[])
{
    int rt = 0; /* return value of function main */
    int fd = -1;
    char buf[2048] = {0};
    //struct sockaddr_in addr = {0};

    //fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_IP));
    //fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    fd = socket(PF_PACKET, SOCK_RAW, IPPROTO_ICMP);
    if (fd < 0) 
    {
        perror("socket");
        return -1;
    }

    while (1)
    {
        if (recvfrom(fd, buf, 2048, 0, NULL, NULL) < 1)
        {
            perror("recvfrom");
        }
        else printf("recv\n");
    }

    close(fd);

    return rt;
}
 
