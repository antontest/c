#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fileio.h>
#include <utils/utils.h>
#include <utils/get_args.h>

int main(int argc, char **argv)
{
    int ret        = 0;
    int help_flag  = 0;
    char *file     = NULL;
    char *appname  = NULL;
    char *keyname  = NULL;
    char *keyvalue = NULL;
    ini_t *ini     = NULL;
    struct options opt[] = {
        {"-f", "--file",     1, RET_STR, ADDR_ADDR(file)},
        {"-a", "--appname",  1, RET_STR, ADDR_ADDR(appname)},
        {"-k", "--keyname",  1, RET_STR, ADDR_ADDR(keyname)},
        {"-v", "--keyvalue", 1, RET_STR, ADDR_ADDR(keyvalue)},
        {"-h", "--help",     0, RET_INT, ADDR_ADDR(help_flag)},
        {NULL}
    };
    struct usage usg[] = {
        {"-f, --file",     "file path"},
        {"-a, --appname",  "appname"},
        {"-k, --keyname",  "keyname"},
        {"-v, --keyvalue", "keyvalue"},
        {"-h, --help",     "show usage"},
        {NULL}
    };

    if (argc < 1) return -1;
    get_args(argc, argv, opt);
    if (help_flag) {
        print_usage(usg);
        return 0;
    }

    ini = ini_create(file);
    if (!ini) return -1;
    if (!keyvalue) {
        keyvalue = ini->get_value(ini, appname, keyname);
        if (keyvalue) {
            printf("%s\n", keyvalue);
        } else {
            printf("ini get value failed\n");
            ret = -1;
        }
    } else {
        ini->set_value(ini, appname, keyname, keyvalue); 
    }
    ini->destroy(ini);
    return ret;
}
