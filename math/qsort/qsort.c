#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct data
{
    int id;
    char sz[30];
};

int comp(const void *p1, const void *p2)
{
    return *(int *)p2 - *(int *)p1;
}

int cmp(const void *p1, const void *p2)
{
    return ((struct data *)p2)->id - ((struct data *)p1)->id;
}

int main()
{
    int arr[]={1,3,5,2,6},i = 0;
    struct data t[5] = {0};
    
    t[0].id=0;
    strcpy(t[0].sz, "2015-05-22 10:22:14.323232");
    t[1].id=0;
    strcpy(t[1].sz, "2015-05-22 10:22:13.323232");
    t[2].id=2;
    strcpy(t[2].sz, "2015-05-22 10:22:12.323232");
    t[3].id=3;
    strcpy(t[3].sz, "2015-05-22 10:22:11.323232");
    t[4].id=4;
    strcpy(t[4].sz, "2015-05-22 10:22:10.323232");

    for(i=0; i<5; i++)
        printf("%s\n",t[i].sz);
    qsort(t, 5, sizeof(struct data), cmp);
    printf("\n");
    
    for(i=0; i<5; i++)
        printf("%s\n",t[i].sz);
    printf("\n");
    
    return 0;
}
