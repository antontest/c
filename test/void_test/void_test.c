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
void test(char *in, void *out)
{
    char *p = in;
    printf("p=%s\n", p);
    int *pp = (int *)out;

    if (in == NULL) return;

    p = strtok(p, ",");
    while (p != NULL) {
        printf("p=%s\n", p);
        //**pp = atoi(p);
        p = strtok(NULL, ";");
        pp++;
    }
}

/*********************************************************
 ******************    Main Function    ******************
 *********************************************************/
int main(int agrc, char *agrv[])
{
    int rt = 0; /* return value of function main */
    int *arr[10] = {0};
    int **p = arr;
    int i = 0;

    test("1,2,3,4", (void *)p);

    for (i = 0; i < 4; i++)
        printf("%d ", *arr[i]);
    printf("\n");

    return rt;
}
