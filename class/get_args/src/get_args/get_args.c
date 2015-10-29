#include <get_args.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief parser agruement from  command line
 *
 * @param agrc   [in]  agruement count
 * @param agrv[] [in]  pointer to agruement
 * @param opt    [out] options struct
 */
void get_args(int agrc, char *agrv[], struct options *opt)
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
