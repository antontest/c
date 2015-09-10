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

void print_list(struct list *node)
{
    plist p = node;

    while (p != NULL) 
    {
        printf("%d\n", p->data.i);
        p = p->next;
    }

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

/*********************************************************
 ******************    Main Function    ******************
 *********************************************************/
int main(int agrc, char *agrv[])
{
    int rt = 0; /* return value of function main */
    plist h = NULL;
    int num = 99;

    h = list_add(h, &num, sizeof(int));
    num = 100;
    h = list_add(h, &num, sizeof(int));
    num = 102;
    h = list_add(h, &num, sizeof(int));
    num = 101;
    h = list_add(h, &num, sizeof(int));
    printf("sort before:\n");
    print_list(h);
    h = bubble_sort(h);
    printf("sort after:\n");
    print_list(h);
    del_list(h);

    return rt;
}

