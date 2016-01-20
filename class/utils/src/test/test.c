/**
 * usual head files
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stack.h>
#include <queue.h>
#include <sort.h>

/*********************************************************
 **************    Function Declaration    ***************
 *********************************************************/
struct ele {
    int id;
    char *name;
};

/*********************************************************
 ******************    Main Function    ******************
 *********************************************************/
int main(int agrc, char *agrv[])
{
    int rt = 0; /* return value of function main */
    stack_t *t = create_stack();
    struct ele e[] = {
        {1, "num1"},
        {2, "num2"},
        {3, "num3"}
    };
    void *item = NULL;

    t->push(t, &e[0]);
    t->push(t, &e[2]);
    t->push(t, &e[1]);
    int num = 101;
    t->push(t, &num);

    printf("stack count: %d\n", t->get_count(t));
    t->remove(t, &e, NULL);

    printf("stack count: %d\n", t->get_count(t));
    t->pull(t, (void **)&item);
    printf("num: %d\n", *(int *)item);
    t->pull(t, (void **)&item);
    printf("rt: %d\n", *(int *)item);
    //t->pull(t, (void **)&r);
    //printf("id: %d, name: %s\n", r->id, r->name);
    printf("stack count: %d\n", t->get_count(t));

    
    t->destroy(t);

    return rt;
}
