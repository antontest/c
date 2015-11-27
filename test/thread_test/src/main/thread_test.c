/**
 * usual head files
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <thread/thread.h>

/*********************************************************
 **************    Function Declaration    ***************
 *********************************************************/
void sayhi()
{
    printf("hi\n");
}

/*********************************************************
 ******************    Main Function    ******************
 *********************************************************/
int main(int agrc, char *agrv[])
{
    int rt = 0; /* return value of function main */
    threads_init();
    thread_t *t = thread_create((void *)sayhi, NULL);
    sleep(1);
    t->cancel(t);
    t->destroy(t);
    threads_deinit();

    return rt;
}
