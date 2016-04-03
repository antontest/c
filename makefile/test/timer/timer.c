/**
 * usual head files
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <thread/thread.h>
#include <timer/timer.h>
#include <tmr/tmr.h>

/*********************************************************
 **************    Function Declaration    ***************
 *********************************************************/
void hi(void *arg)
{
    printf("%s\n", (char *)arg);
}

/*********************************************************
 ******************    Main Function    ******************
 *********************************************************/
int main(int agrc, char *agrv[])
{
    int rt = 0; /* return value of function main */

    tmr_arg_t arg;
    tmr_arg_t arg1;
    tmr_t *tmr = NULL;
    tmr = tmr_create();

    arg.wait = 800;
    arg.arg = "hi tmr 1";
    arg.cb = hi;
    arg.cnt = 10;

    arg1.wait = 2000;
    arg1.arg = "hi tmr 2";
    arg1.cb = hi;
    arg1.cnt = 5;

    tmr->start(tmr, &arg);
    tmr->start(tmr, &arg1);
    sleep(10);
    tmr->destroy(tmr);
    
    return 0;

    timer *t;
    t = timer_start(hi, "hi", 100);
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
