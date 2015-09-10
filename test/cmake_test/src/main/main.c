/**
 * usual head files
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <hello.h>

/*********************************************************
 **************    Function Declaration    ***************
 *********************************************************/
int orstr(const char *str, int ret)
{
    printf("%s\n", str);
    return ret;
}

/*********************************************************
 ******************    Main Function    ******************
 *********************************************************/
int main(int agrc, char *agrv[])
{
    int rt = 0; /* return value of function main */
    char a[2] = "1";
    char b[2] = "0";
    int aa = 0;
    int bb = 0;
    //int cc = 0;

    phello();
    if ((aa=atoi(a) == 1) && (bb=atoi(b) == 1))
    printf("hello world\n");
    if (orstr("or 11", 0) || orstr("or 22", 0) || orstr("or 33", 1) || orstr("or 44", 1)) printf("or 1\n");
    if (orstr("and 11", 0) && orstr("and 22", 0) && orstr("and 33", 1) && orstr("and 44", 1)) printf("and 1\n");

    return rt;
}
