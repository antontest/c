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
    int pid = -1;
    char buf[128] = {0};
    FILE *fp = NULL;

    if ((fp = fopen("t.txt", "r")) == NULL) return -1;
    while (fgets(buf, sizeof(buf), fp) != NULL)
    {
        if (strstr(buf, "Pid") != NULL)
        {
            printf("11: %s\n", buf);
            break;
        }
    }
    if (fscanf(fp, "PPid:%d", &pid) == -1) return -1;
    printf("ppid: %d\n", pid);
    if (fp != NULL )fclose(fp);

    return rt;
}
