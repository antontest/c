#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>

/*********************************************************
 **************    Function Declaration    ***************
 *********************************************************/

/*********************************************************
 ******************    Main Function    ******************
 *********************************************************/
int main(int agrc, char *agrv[])
{
    int rt = 0; /* return value of function main */

    if (agrc < 2) return -1;
    if (agrv[1] != NULL)
        rt = atoi(agrv[1]);
    kill(-rt, SIGTERM);

    return rt;
}
