/**
 * usual head files
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <get_args.h>

/*********************************************************
 **************    Function Declaration    ***************
 *********************************************************/

/*********************************************************
 ******************    Main Function    ******************
 *********************************************************/
int main(int agrc, char *agrv[])
{
    int rt = 0; /* return value of function main */
    int help_flag = 0;
    char *ip = NULL;
    int port = 0;
    char c = '0';
    char s = '0';
    struct options opts[] = {
        {"-h", "--help", 0, RET_INT   , (void **)&help_flag},
        {"-i", "--ip"  , 1, RET_STRING, (void **)&ip},
        {"-p", "--port", 1, RET_INT   , (void **)&port},
        {"-c", "--char", 1, RET_CHAR  , (void **)&c},
        {"-s", "--stat", 0, RET_CHAR  , (void **)&s},
        {NULL, NULL    , 0, 0         , NULL},
    };

    get_args(agrc, agrv, opts);

    printf("help_flag: %d\n", help_flag);
    if (ip != NULL) printf("ip: %s\n", ip);
    printf("port: %d\n", port);
    printf("char: %c\n", c);
    printf("stat: %c\n", s);

    return rt;
}
