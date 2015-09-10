#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <sys/sysinfo.h>
#include <sys/errno.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <getopt.h>
#include <time.h>
#include <stdarg.h>

#define add(a,b,...) (a + b + __VA_ARGS__)

/*********************************************************
 **************    Function Declaration    ***************
 *********************************************************/
void test(int a, int b, ...)
{
     //va_list arg;
    int para = 0;
    int *arg = &b;
    char buf[10] = "hello";

    printf("%c\n", buf[-1]);

    //va_start(arg, b);
    //para = va_arg(arg, int);
    //para = *(int *)((unsigned int)&b + sizeof(int));
    para = *(++arg);
    printf("para = %d\n", para);
    if (para < 0) para = 0;
    printf("sum = %d\n", a + b + para);
    //va_end(arg);
    //else printf("sum = %d\n", a + b);
}


/*********************************************************
 ******************    Main Function    ******************
 *********************************************************/
int main(int agrc, char *agrv[])
{
    int rt = 0; /* return value of function main */
    
    test(1, 2, 3);
    //printf("add: %d\n", add(1,2));

    return rt;
}
