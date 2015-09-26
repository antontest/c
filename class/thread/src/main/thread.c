/**
 * usual head files
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "thread.h"
#include "thread_value.h"

/*********************************************************
 **************    Function Declaration    ***************
 *********************************************************/
void* sayhi(void *arg)
{
    sleep(5);
    printf("hi\n");

    return NULL;
}

void cleanup(void *arg)
{
    printf("clean up\n");
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
    thread_cleanup_push(cleanup, NULL);

    sleep(1);
    thread->cancel(thread);
    thread_cleanup_pop(true);
    sleep(2);
    threads_deinit();
    free(thread);

    return rt;
}
