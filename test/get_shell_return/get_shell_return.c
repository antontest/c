#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*********************************************************
 *****************    Macro Defination    ****************
 *********************************************************/

/*********************************************************
 **************    Function Declaration    ***************
 *********************************************************/
int get_shell_return(const char *cmd)
{
    char result[1024] = {0};
    FILE *fpRead;
    char buf[1024] = {0};

    if (cmd == NULL) return -1;
    fpRead = popen(cmd, "r");
    while(fgets(buf,1024-1,fpRead)!=NULL)
    { 
        //result = buf;
        strcat(result, buf);
        if (strlen(buf) > 0)
        printf("result: %s", result);
    }
    if(fpRead!=NULL)
        pclose(fpRead);
    //printf("result: %s\n", result);
    return 0;
}

/*********************************************************
 ******************    Main Function    ******************
 *********************************************************/
int main(int agrc, char *agrv[])
{
    int rt = 0; /* return value of function main */
    get_shell_return(agrv[1]);

    return rt;
}

