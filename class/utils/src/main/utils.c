#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <library.h>
//#include <utils.h>
//#include <enum.h>
//#include <linked_list.h>
#include <debug.h>
#include <log.h>

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
/*********************************************************
 ******************    Main Function    ******************
 *********************************************************/
int main(int agrc, char *agrv[])
{
    int rt = 0; /* return value of function main */
    struct ele el= {2, NULL};
    void *p = NULL;

    linked_list_t *list = linked_list_create();
    rt = 1;
    list->insert_last(list, &rt);
    list->insert_last(list, &el);
    printf("list count: %d\n", list->get_count(list));
    list->get_first(list, &p);
    printf("first val: %d\n", *((int *)p));
    list->get_last(list, &p);
    printf("lsst val: %d\n", ((struct ele *)p)->data);
    free(list);

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
