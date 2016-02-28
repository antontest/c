/**
 * usual head files
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*********************************************************
 **************    Function Declaration    ***************
 *********************************************************/

/*********************************************************
 ******************    Main Function    ******************
 *********************************************************/
int main(int agrc, char *agrv[])
{
    int rt = 0; /* return value of function main */

    printf("sizeof(char): %d\n", sizeof(char));
    printf("sizeof(unsigned char): %d\n", sizeof(unsigned char));
    printf("sizeof(int): %d\n", sizeof(int));
    printf("sizeof(unsigned int): %d\n", sizeof(unsigned int));
    printf("sizeof(short): %d\n", sizeof(short));
    printf("sizeof(unsigned short): %d\n", sizeof(unsigned short));
    printf("sizeof(long): %d\n", sizeof(long));
    printf("sizeof(unsigned): %d\n", sizeof(unsigned long));
    printf("sizeof(float): %d\n", sizeof(float));
    printf("sizeof(double): %d\n", sizeof(double));

    return rt;
}
