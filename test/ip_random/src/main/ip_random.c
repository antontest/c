/**
 * usual head files
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

/*********************************************************
 **************    Function Declaration    ***************
 *********************************************************/

/*********************************************************
 ******************    Main Function    ******************
 *********************************************************/
int main(int agrc, char *agrv[])
{
    int rt = 0; /* return value of function main */
    unsigned char start_addr[4] = {13, 13, 13, 10};
    unsigned char end_addr[4] = {13, 13, 13, 25};
    unsigned char new_addr[4] = {0};

    unsigned int end, start;
    start = start_addr[0] * 256 * 256 * 256 + start_addr[1] * 256 * 256 + start_addr[2] * 256 + start_addr[3];    
    end = end_addr[0] * 256 * 256 * 256 + end_addr[1] * 256 * 256 + end_addr[2] * 256 + end_addr[3];
    srandom(time(NULL));
    unsigned int num = random() % (end - start) + start;
    new_addr[0] = num / 256 / 256 / 256;
    new_addr[1] = num / 256 / 256;
    new_addr[2] = num / 256;
    new_addr[3] = num % 256;
    printf("new_addr: %d:%d:%d:%d\n",
        new_addr[0],
        new_addr[1],
        new_addr[2],
        new_addr[3]
        );

    return rt;
}
