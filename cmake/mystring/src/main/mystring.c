/**
 * usual head files
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <mystring.h>

/*********************************************************
 **************    Function Declaration    ***************
 *********************************************************/

/*********************************************************
 ******************    Main Function    ******************
 *********************************************************/
int main(int agrc, char *agrv[])
{
    int rt = 0; /* return value of function main */

    printf("%s\n", int2string(1));
    printf("%s\n", int2string(2));
    unsigned char ip[4] = {192, 168, 1, 1};
    unsigned char mac[6] = {0x00, 0x80, 0x8b, 0xb8, 0x88, 0x99};

    printf("%s\n", ip2string(ip));
    printf("%s\n", mac2string(mac));
    printf("%s\n", format2string("%d.%d", 1, 2));
    
    return rt;
}
