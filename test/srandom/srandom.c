/**
 * usual head files
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

/*********************************************************
 **************    Function Declaration    ***************
 *********************************************************/
int rand_num(int min, int max)
{
    //srandom((unsigned int)time(NULL));
    //return (ret = random() % max) < min ? min + ret : ret;
    return random() % (max - min) + min;
}


/*********************************************************
 ******************    Main Function    ******************
 *********************************************************/
int main(int agrc, char *agrv[])
{
    int rt = 0; /* return value of function main */
    
    srandom((unsigned int)time(NULL));
    printf("rand num: \n");
    while (rt++ < 100)
        printf("%d ", rand_num(10,50));
        //printf("rand num: %d\n", rand_num(10,50));
    printf("\n");

    return rt;
}
