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

void parser_args(int agrc, char *agrv[], struct options *opt)
{
    int i = 0;
    int ret_int = 1;
    int fount_flag = 0;
    struct options *opts = opt;

    if (agrc < 1 || agrv == NULL) {
        fprintf(stderr, "No parameter to parser!\n");
        exit(1);
    }
    if (opt == NULL) {
        fprintf(stderr, "Please give struct options!\n");
        exit(1);
    }

    for (i = 1; i < agrc; i++) {
        fount_flag = 0;
        for (opts = opt; (opts->short_name != NULL || opts->long_name != NULL); opts++) {
            if (opts->been_get == 1) {
                fount_flag = 1;
                continue;
            }
             
            if ((opts->short_name != NULL && strlen(agrv[i]) == strlen(opts->short_name) && !strncasecmp(opts->short_name, agrv[i], strlen(opts->short_name))) || (opts->long_name != NULL && strlen(agrv[i]) == strlen(opts->long_name) && !strncasecmp(opts->long_name, agrv[i], strlen(opts->long_name)))) {
    
                fount_flag = 1;
                if (opts->value == NULL) continue;
                if (opts->has_args == 0) {
                    ret_int = 1;
                    memcpy(opts->value, &ret_int, sizeof(ret_int));
                } else {
                    if (++i >= agrc) {
                        fprintf(stderr, "agruement [%s] need a parameter\n", agrv[i - 1]);
                        exit(1);
                    }

                    switch (opts->value_type) {
                        case RET_CHAR:
                            memcpy(opts->value, agrv[i], 1);
                        case RET_INT:
                            if (i < agrc) {
                                ret_int = atoi(agrv[i]);
                            }
                            memcpy(opts->value, &ret_int, sizeof(int));
                            break;
                        default:
                            *(opts->value) = agrv[i];
                            break;
                    }
                }

                opts->been_get = 1;
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

    parser_args(agrc, agrv, opts);
    printf("help_flag: %d\n", help_flag);
    if (ip != NULL) printf("ip: %s\n", ip);
    printf("port: %d\n", port);

    return rt;
}
