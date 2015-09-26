#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <utils.h>
#include <enum.h>
#include <linked_list.h>

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

    return rt;
}
