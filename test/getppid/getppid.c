#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*********************************************************
 **************    Function Declaration    ***************
 *********************************************************/
int get_parent_proc_name()
{
    char buf[64] = {0};
    char cmd[64] = {0};
    FILE *fp = NULL;
    pid_t pgrp_id = getpgrp();

    printf("%d\n", pgrp_id);
    sprintf(cmd, "ps -a | grep -v ps | grep %d | awk '{ print $4 }'", pgrp_id);
    if ((fp = popen(cmd, "r")) == NULL) return -1;
    
    if(fgets(buf, sizeof(buf), fp) != NULL) ;
    printf("name: %s\n", buf);

    if (strlen(buf) > 0) return 0;
    return -1;
}

/*********************************************************
 ******************    Main Function    ******************
 *********************************************************/
int main(int agrc, char *agrv[])
{
    int rt = 0; /* return value of function main */

    get_parent_proc_name();
    sleep (30);

    return rt;
}
