/**
 * usual head files
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <tm.h>

/*********************************************************
 **************    Function Declaration    ***************
 *********************************************************/

/*********************************************************
 ******************    Main Function    ******************
 *********************************************************/
int main(int agrc, char *agrv[])
{
    int rt = 0; /* return value of function main */
    struct tm *now = NULL;
    struct timespec *tp = NULL;
    now = real_sys_time();
    printf("local time: %s", asctime(now));

    run_time_start();
    sleep(1);
    usleep(10);
    tp = run_time_end();
    printf("run: %lds, %ldns\n", tp->tv_sec, tp->tv_nsec);
    sleep(1);
    tp = run_time_end();
    printf("run: %lds, %ldns\n", tp->tv_sec, tp->tv_nsec);
    

    return rt;
}
