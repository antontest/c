#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*********************************************************
 *****************    Macro Defination    ****************
 *********************************************************/
typedef struct list * plist;

typedef union list_data 
{
    char c;
    int i;
    void *p;
} list_data;

typedef struct list
{
    list_data data;

    plist next;
} list_t;

/*********************************************************
 **************    Function Declaration    ***************
 *********************************************************/
void value_set(struct list *node, void *data, int size)
{
    memcpy(&node->data, data, size);
}

void print_list(struct list *node, const char *pre)
{
    plist p = node;

    printf("%s\n  ", pre);
    while (p != NULL) 
    {
        printf("%d ", p->data.i);
        p = p->next;
    }
    printf("\n");

    return;
}

struct list * list_malloc()
{
    plist list = (plist)malloc(sizeof(struct list));
    if (list == NULL) return NULL;

    bzero(list, sizeof(struct list));
    return list;
}

int del_list(struct list *list_head)
{
    plist p = list_head;
    if (p == NULL) return -1;
    
    while (p != NULL)
    {
        free(p);
        p = p->next;
        //printf("del list\n");
    }

    return 0;
}

struct list * list_add(struct list *list_head, void *data, int size)
{
    plist p = list_head;
    plist pnew = NULL;
    
    while (p != NULL && p->next != NULL) p = p->next;
    pnew = list_malloc();
    value_set(pnew, data, size);
    
    pnew->next = NULL;
    if (list_head == NULL) list_head = pnew;
    else p->next = pnew;
    
    return list_head;
}

struct list * bubble_sort(struct list *list_head)
{
    plist pre = NULL;
    plist cur = list_head;
    plist next = NULL;
    plist pt = NULL;
    struct list h;
    int flag = 1;

    if (cur == NULL) return list_head;
    
    h.next = list_head;
    pre = &h;
    cur = pre->next;
    next = cur->next;

    while (h.next != pt && flag)
    {
        pre = &h;
        cur = pre->next;
        next = cur->next;

        flag = 0;
        while (next != pt)
        {
            if (cur->data.i > next->data.i)
            {
                cur->next = next->next;
                pre->next = next;
                next->next = cur;

                pre = next;
                next = cur->next;

                flag = 1;
            }
            else
            {
                pre = cur;
                cur = next;
                next = next->next;
            }
        }

        pt = cur;
    }

    return h.next;
}

struct list * select_sort(struct list *l)
{
    plist p = l;
    plist head = NULL;
    plist tail = NULL;
    plist min = NULL;
    plist min_pre = NULL;

    while (l != NULL)
    {
        for (p = l, min = l; p->next != NULL; p = p->next)
        {
            if (min->data.i > p->next->data.i)
            {
                min_pre = p;
                min = p->next;
            }
        }
        
        if (head == NULL) head = min;
        else tail->next = min;
        tail = min;

        if (min == l) l = l->next;
        else min_pre->next = min->next;
    }

    if (head != NULL) tail->next = NULL;

    return (l = head);
}

struct list * insert_sort(struct list *l)
{
    plist p = l, pre = NULL, q = NULL, head = l->next;
    p->next = NULL;

    while (head != NULL)
    {
        head = head->next;
        pre = l;
        p = l->next;
        while (p != NULL && p->data.i < ->data.i)
        {

        }
    }
}

/*********************************************************
 ******************    Main Function    ******************
 *********************************************************/
int main(int agrc, char *agrv[])
{
    int rt = 0; /* return value of function main */
    plist h = NULL;
    int num = 0;

    num = 98;
    h = list_add(h, &num, sizeof(int));
    num = 100;
    h = list_add(h, &num, sizeof(int));
    num = 98;
    h = list_add(h, &num, sizeof(int));
    num = 101;
    h = list_add(h, &num, sizeof(int));

    print_list(h, "before:");
    h = select_sort(h);
    //h = bubble_sort(h);
    print_list(h, "after:");
    
    del_list(h);

    return rt;
}

