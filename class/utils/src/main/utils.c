#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <library.h>
//#include <utils.h>
//#include <enum.h>
#include <linked_list.h>
#include <debug.h>
#include <log.h>
#include <get_args.h>

/*********************************************************
 **************    Function Declaration    ***************
 *********************************************************/
typedef struct ele {
    int data;
    struct ele *p;
} ele;

typedef enum st1 {
    ST_1,
    ST_2,
    ST_3
} st1;

ENUM(st, ST_1, ST_3,
    "ST1",
    "ST2",
    "ST3")

void list_test(linked_list_t *list)
{
    struct ele el= {2, NULL};
    int rt = 1;
    void *p = NULL;

    list->insert_last(list, &rt);
    list->insert_last(list, &el);
    printf("list count: %d\n", list->get_count(list));
    list->get_first(list, &p);
    printf("first val: %d\n", *((int *)p));
    list->get_last(list, &p);
    printf("lsst val: %d\n", ((struct ele *)p)->data);
    //free(list);

}
/*********************************************************
 ******************    Main Function    ******************
 *********************************************************/
int main(int agrc, char *agrv[])
{
    int rt = 0; /* return value of function main */
    int help_flag = 0;
    //void *p = NULL;
    char *ip = NULL;
    int port = 0;
    int num[3];
    char *files = {NULL};
    char **pp = &files;
    struct usage use[] = {
        {"-h, --help", "help"},
        {"-p, --passwd", "pass word pass word pass word pass word abcdefghijklmnopqrstuvwxyz"},
        {NULL, NULL}
    };
    struct options opts[] = {
        {"-h", "--help" , 0, RET_INT, ADDR_ADDR(help_flag)},
        {"-p", "--port" , 1, RET_INT, ADDR_ADDR(port)},
        {"-i", "--ip"   , 1, RET_STR, ADDR_ADDR(ip)},
        {"-n", "--num"  , 3, RET_INT, ADDR_ADDR(num)},
        {"-f", "--files", -1, RET_STR, ADDR_ADDR(files)},
        {NULL, NULL}
    };

    printf("172.21.34.211/31 -- %s\n", ip_netmask("172.21.34.211", 31));
    if (0) {
    get_args(agrc, agrv, opts);
    printf("ip: %s\n", ip);
    printf("port: %d\n", port);
    printf("value: %d, val1: %d, val2: %d\n", num[0], num[1], num[2]);
    //printf("f1: %s, f2: %s, f3: %s\n", *files, *files, *files);
    //printf("f1: %s, f2: %s, f3: %s\n", *pp, *(pp+1), *(pp+2));
    pp += 2;
    printf("11111\n");
    if (*pp == NULL) printf("NULL");
    printf("22222\n");
    //printf("*(pp + 2): %s\n", *pp);

    if (help_flag) print_usage(use);
    return 0; 
    }

    /*
    linked_list_t *list = linked_list_create();
    struct ele el= {2, NULL};
    rt = 1;

    list->insert_last(list, &rt);
    list->insert_last(list, &el);
    printf("list count: %d\n", list->get_count(list));
    list->get_first(list, &p);
    printf("first val: %d\n", *((int *)p));
    list->get_last(list, &p);
    printf("lsst val: %d\n", ((struct ele *)p)->data);
    */
    /*
    list_test(list);
    printf("list count: %d\n", list->get_count(list));
    list->get_first(list, &p);
    printf("first val: %d\n", *((int *)p));
    list->get_last(list, &p);
    printf("lsst val: %d\n", ((struct ele *)p)->data);
    free(list);
    return 0;
    */

    printf("name: %s\n", enum_to_name(st, ST_2));
    dbg("%d aaa bbb %s", 1, "ccc");
    dbg_pos("%d aaa bbb %s", 2, "ccc");
    dbg_err(DBG_LOG, "%d aaa bbb %s", 3, "ccc");
    dbg2("%d aaa bbb %s", 3, "ccc");

    /*
    log_t *log;
    log = log_create("log.log");
    log->log(log, DBG_NET, LEVEL_DEBUG, "This is a test");
    log->destroy(log);
    */

    log_init(NULL);
    log_error(DBG_NET, "%d This is a test %s", 1, "1");
    log_warn(DBG_NET, "%s This is a test", "aaaaa");
    log_deinit();

    return rt;
}
