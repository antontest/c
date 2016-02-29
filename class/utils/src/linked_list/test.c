#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <linked_list.h>

void print_list(linked_list_t *this)
{
    int *num = NULL;
    this->reset_enumerator(this);
    while (this->enumerate(this, (void **)&num)) {
        printf("%d ", *num);
    }
    printf("\n");
}

int cmp (void *a, void *b)
{
    int ia = *(int *)a;
    int ib = *(int *)b;

    return ia > ib ? 1 : 0;
}

void print(int *el)
{
    printf("%d ", *el);
}

int main(int argc, char **argv)
{
    linked_list_t *list = NULL;
    int a = 6, b = 2, c = 3, d = 11;

    list = linked_list_create();
    if (!list) return -1;

    list->insert_last(list, &a);
    list->insert_last(list, &b);
    list->insert_last(list, &c);
    list->insert_last(list, &d);
    print_list(list);
    list->bubble(list, cmp);
    list->print(list, (void *)print);
    printf("\n");

    list->destroy(list);

    return 0;
}
