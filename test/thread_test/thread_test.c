#include <thread.h>

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
void* echo(void *arg)
{
    //while (1)
    {
        printf("echo %s\n", (char *)arg);
        //usleep(100000);
    }

    return NULL;
}

/*********************************************************
 ******************    Main Function    ******************
 *********************************************************/
int main(int agrc, char *agrv[])
{
    int rt = 0; /* return value of function main */
    thread_t t = {0};

    pthread_start(&t, echo, "1", 1, 0);
    //sleep(1);
    //pthread_delete(&t);
    pthread_time_wait_over(&t, 1000);

    return rt;
}
