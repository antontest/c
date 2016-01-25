/**
 * usual head files
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <thread.h>
#include <timer.h>

/*********************************************************
 **************    Function Declaration    ***************
 *********************************************************/
void hi(void *arg)
{
    printf("hi\n");
}

/*********************************************************
 ******************    Main Function    ******************
 *********************************************************/
int main(int agrc, char *agrv[])
{
    int rt = 0; /* return value of function main */
    timer *t;
    t = timer_start(hi, NULL, 100);
    t->start(t);
    printf("timer state1: %s\n", t->get_state_str(t));
    sleep(1);
    t->pause(t);
    t->set_interval(t, 1000);
    printf("timer run times: %d\n", t->get_runtimes(t));
    printf("timer state: %s\n", t->get_state_str(t));
    sleep(1);
    t->resume(t);
    sleep(2);
    t->destroy(t);

    return rt;
}
