/**
 * usual head files
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <thread.h>

/*********************************************************
 **************    Function Declaration    ***************
 *********************************************************/
void* sayhello(void *arg)
{
    printf("hello\n");

    return NULL;
}

void* sayhi(void *arg)
{
    printf("hi\n");

    return NULL;
}

void* waitsleep(void *arg)
{
    sleep(2);
    printf("sleep 2\n");
    return NULL;
}

void* waitsleep1(void *arg)
{
    sleep(4);
    printf("sleep 2\n");

    return NULL;
}

void clean(void *arg)
{
    printf("clean up\n");
}

void *pthread(void *arg)
{
    printf("pthrad\n");
    //struct thread *pthread = (struct thread *)arg;
    //printf("d: %d, r: %d\n", pthread->delete, pthread->run);

    /*
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    pthread_cleanup_push(clean, NULL);
    pthread_testcancel();
    */
    sleep(5);
    printf("pthread sleep 5\n");
    //pthread_testcancel();

    //printf("pthread be canceled\n");
    //pthread_cleanup_pop(0);
    return NULL;
}

/*********************************************************
 ******************    Main Function    ******************
 *********************************************************/
int main(int agrc, char *agrv[])
{
    int rt = 0; /* return value of function main */
            
    return rt;
}
