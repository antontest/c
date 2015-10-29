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
typedef enum ret_value_type{
    RET_CHAR   = 0,
    RET_INT    = 1,
    RET_STRING = 2
} ret_value_type;
struct options {
    const char *short_name;
    const char *long_name;
    const int  has_args;
    const int  value_type;
    void       **value;
    int        been_get;
};

void parser_args(int agrc, char *agrv[], struct options opt[])
{
    int i = 0;
    int j = 0;
    int ret_int = 1;
    int fount_flag = 0;
    printf("1111111111111111111\n");
    struct options **opts = (struct options **)&opt;
    printf("2222222222222222222\n");
    //printf("short_name: %s\n", (opt++)->short_name);
    //printf("short_name: %s\n", (opt++)->short_name);
    //printf("short_name: %s\n", (opt++)->short_name);
    struct options *p = opt;
    while (p->short_name != NULL || p->long_name != NULL) {
         printf("short_name: %s\n", p->short_name);
         p++;
    }

    for (i = 1; i < agrc; i++) {

        fount_flag = 0;
        printf("2222222222222222222\n");
        for (j = 0; (opts[j]->short_name != NULL || opts[j]->long_name != NULL); j++) {
            if (opts[j]->been_get == 1) {
                fount_flag = 1;
                continue;
            }
             
            printf("short_name: %s, argv: %s\n", opts[j]->short_name, agrv[i]);
            if ((opts[j]->short_name != NULL && strlen(agrv[i]) == strlen(opts[j]->short_name) && !strncasecmp(opts[j]->short_name, agrv[i], strlen(opts[j]->short_name))) || (opts[j]->long_name != NULL && strlen(agrv[i]) == strlen(opts[j]->long_name) && !strncasecmp(opts[j]->long_name, agrv[i], strlen(opts[j]->long_name)))) {
    
                fount_flag = 1;
                if (opts[j]->value == NULL) continue;
                if (opts[j]->has_args == 0) {
                    ret_int = 1;
                    memcpy(opts[j]->value, &ret_int, sizeof(ret_int));
                    printf("3333333333333333333333333\n");
                } else {
                    if (++i >= agrc) {
                        fprintf(stderr, "agruement [%s] need a parameter\n", agrv[i - 1]);
                        exit(1);
                    }

                    switch (opts[j]->value_type) {
                        case RET_CHAR:
                            memcpy(opts[j]->value, agrv[i], 1);
                        case RET_INT:
                            if (i < agrc) {
                                ret_int = atoi(agrv[i]);
                            }
                            memcpy(opts[j]->value, &ret_int, sizeof(int));
                            break;
                        default:
                            *(opts[j]->value) = agrv[i];
                            break;
                    }
                }

                opts[j]->been_get = 1;
                break;
            } 
        }

        if (!fount_flag) {
            fprintf(stderr, "Invalid arguement\n");
            exit(1);
        }
    }


    return;

}

/*********************************************************
 ******************    Main Function    ******************
 *********************************************************/
int main(int agrc, char *agrv[])
{
    int rt = 0; /* return value of function main */

    int help_flag = 0;
    char *ip = NULL;
    int port = 0;
    struct options opts[] = {
        {"-h", "--help", 0, RET_INT, (void **)&help_flag },
        {"-i", "--ip", 1, RET_STRING, (void **)&ip },
        {"-p", "--port", 1, RET_INT, (void **)&port },
        {"-t", "--t", 1, RET_INT, NULL },
        {NULL, NULL, 0, 0, NULL}
    };

    //struct options **p = (struct options **)&opts;
    //printf("name: %s\n", p[0]->short_name);
    parser_args(agrc, agrv, opts);

    printf("help_flag: %d\n", help_flag);
    if (ip != NULL) printf("ip: %s\n", ip);
    printf("port: %d\n", port);
    return rt;
}
