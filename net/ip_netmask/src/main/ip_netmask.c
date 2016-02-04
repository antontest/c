/**
 * usual head files
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/*********************************************************
 **************    Function Declaration    ***************
 *********************************************************/

/*********************************************************
 ******************    Main Function    ******************
 *********************************************************/
int main(int agrc, char *agrv[])
{
    int rt = 0; /* return value of function main */

    struct in_addr addr;
    unsigned long u_ip = inet_addr("172.21.34.167");
    int netmask = 16;
    
    int len = sizeof(u_ip) * 8;
    printf("len: %d\n", len);
    while (netmask < len) {
        u_ip &= ~(1 << netmask++);
    }
    memcpy(&addr, &u_ip, sizeof(addr));
    printf("netmask ip: %s\n", inet_ntoa(addr));

    return rt;
}
