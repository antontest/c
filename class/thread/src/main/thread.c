/**
 * usual head files
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "thread.h"

/*********************************************************
 **************    Function Declaration    ***************
 *********************************************************/
void* sayhi(void *arg)
{
    sleep(5);
    printf("hi\n");

    return NULL;
}

/*********************************************************
 ******************    Main Function    ******************
 *********************************************************/
int main(int agrc, char *agrv[])
{
    int rt = 0; /* return value of function main */

    thread_t *thread = NULL;
    threads_init();
    thread_cancelability(true);
    thread = thread_create(sayhi, NULL);

    sleep(1);
    thread->cancel(thread);
    threads_deinit();
    free(thread);

    return rt;
}
