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
    printf("hi\n");

    return NULL;
}

/*********************************************************
 ******************    Main Function    ******************
 *********************************************************/
int main(int agrc, char *agrv[])
{
    int rt = 0; /* return value of function main */

    thread_t *thread = NULL, *t = NULL;
    threads_init();
    thread = thread_create(sayhi, NULL);
    t = thread_create(sayhi, NULL);

    sleep(1);
    threads_deinit();
    free(thread);
    free(t);

    return rt;
}
