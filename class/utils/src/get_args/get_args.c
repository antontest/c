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
    int found_flag = 0;
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
        found_flag = 0;
        for (opts = opt; (opts->short_name != NULL || opts->long_name != NULL); opts++) {
            if (opts->been_get == 1) {
                found_flag = 1;
                continue;
            }
             
            if ((opts->short_name != NULL && strlen(agrv[i]) == strlen(opts->short_name) && !strncasecmp(opts->short_name, agrv[i], strlen(opts->short_name))) || (opts->long_name != NULL && strlen(agrv[i]) == strlen(opts->long_name) && !strncasecmp(opts->long_name, agrv[i], strlen(opts->long_name)))) {
    
                found_flag = 1;
                if (opts->value == NULL) continue;
                if (opts->has_args == 0) {
                    if (opts->value_type == RET_STR) {
                        fprintf(stderr, "option [%s] return value type wrong\n", agrv[i]);
                        exit(1);
                    }

                    if (opts->value_type == RET_INT) {
                        ret_int = 1;
                        memcpy(opts->value, &ret_int, sizeof(int));
                    } else {
                        memcpy(opts->value, "1", 1);
                    }
                } else {
                    if (++i >= agrc) {
                        fprintf(stderr, "agruement [%s] need a parameter\n", agrv[i - 1]);
                        exit(1);
                    }

                    switch (opts->value_type) {
                        case RET_CHR:
                            memcpy(opts->value, agrv[i], 1);
                            break;
                        case RET_INT:
                            if (i < agrc) ret_int = atoi(agrv[i]);
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

        if (!found_flag) {
            fprintf(stderr, "Invalid arguement\n");
            exit(1);
        }
    }

    return;

}
