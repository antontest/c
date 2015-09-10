#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

/*********************************************************
 *****************    Macro Defination    ****************
 *********************************************************/

/*********************************************************
 **************    Function Declaration    ***************
 *********************************************************/
int match_ip(const char *ip)
{
    int ret = -1;
    int count = 0;
    const char *p = ip;
    struct in_addr addr = {0};
    
    if (p == NULL) return -1;
    while (*p != '\0')
    {
        if (*p == '.') count++;
        p++;
    }

    if (count != 3) return -1;

    addr.s_addr = inet_addr(ip);
    ret = inet_addr(inet_ntoa(addr));

    if (ret == -1) return -1;
    else return 0;
}

/*********************************************************
 ******************    Main Function    ******************
 *********************************************************/
int main(int agrc, char *agrv[])
{
    int rt = 0; /* return value of function main */

    printf("ip: %s\n", agrv[1]);
    if (!match_ip(agrv[1])) printf("Yes\n");
    else printf("No\n");

    return rt;
}

