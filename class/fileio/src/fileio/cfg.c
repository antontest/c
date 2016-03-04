#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <utils/utils.h>
#include <utils/get_args.h>
#include <fileio.h>

int main(int argc, char **argv)
{
    int ret        = 0;
    int help_flag  = 0;
    char *file     = NULL;
    char *keyname  = NULL;
    char *keyvalue = NULL;
    struct options opt[] = {
        {"-f", "--file",     1, RET_STR, ADDR_ADDR(file)},
        {"-k", "--keyname",  1, RET_STR, ADDR_ADDR(keyname)},
        {"-v", "--keyvalue", 1, RET_STR, ADDR_ADDR(keyvalue)},
        {"-h", "--help",     0, RET_INT, ADDR_ADDR(help_flag)},
        {NULL}
    };
    struct usage usg[] = {
        {"-f, --file",     "configuration file"},
        {"-k, --keyname",  "key name"},
        {"-v, --keyvalue", "key value"},
        {"-h, --help",     "show usage"},
        {NULL}
    };
    cfg_t *cfg = NULL;
 
    get_args(argc, argv, opt);
    if (help_flag) {
        print_usage(usg);
        return 0;
    }
    
    cfg = cfg_create(file);
    if (!cfg) return -1;
    if (!keyvalue) {
        keyvalue = (char *)malloc(256);
        if (!keyvalue) return -1;
        keyvalue = cfg->get_value(cfg, keyname, keyvalue, 256);
        if (keyvalue) printf("%s\n", keyvalue);
        else {
            printf("cfg get value failed\n");
            ret = -1;
        }
    } else {
        ret = cfg->set_value(cfg, keyname, keyvalue);
    }
 
    return ret;
}
