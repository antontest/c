#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int read_config(int num,const char *path)
{
    FILE *fp = NULL;
    int i = 0;
    char buf[1024] = {0};
    char name_buf[128] = {0};
    char *name = NULL, *value = NULL;

    if (NULL == path) {
        printf("path is null.\n");
        return -1;
    }

    if ((fp = fopen(path, "r")) == NULL) {
        printf("fopen failed.\n");
        fclose(fp);
        return -1;
    }


    while(fgets(buf, 1024, fp) != NULL) {
        name = strtok(buf, "=");
        value = strtok(NULL, "=");
        if (name == NULL || value == NULL) continue;

        sprintf(name_buf, "task_q[%d].task_q_time_out", i);
        if (strncmp(name, name_buf, strlen(name)) == 0) {
            if (i >= num) continue;
            printf("value = %s", value);
            i++;
        } else if (strncmp(name, "select_time_out", strlen(name)) == 0) {
            printf("value = %s", value);
        }
    }

    return 0;
}

int main(int agrc, char *agrv[])
{
    if (agrc < 3) {
        printf("param is not valid.\n");
        return -1;
    }

    read_config(atoi(agrv[1]), agrv[2]);
    
    return 0;
}
