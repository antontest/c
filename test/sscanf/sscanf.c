/**
 * usual head files
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*********************************************************
 **************    Function Declaration    ***************
 *********************************************************/

/*********************************************************
 ******************    Main Function    ******************
 *********************************************************/
int main(int agrc, char *agrv[])
{
    int rt = 0; /* return value of function main */
    char buf[128] = {0};
    char *p = agrv[1];
    if (p != NULL)
    {
        while (*p != '\0') p++;
        sscanf(agrv[1], "{{%s}" , buf);
    }
    if (strlen(buf)) printf("%s\n", buf);

    return rt;
}
