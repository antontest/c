#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*********************************************************
 *****************    Macro Defination    ****************
 *********************************************************/

/*********************************************************
 **************    Function Declaration    ***************
 *********************************************************/
int select_sort(int a[], int size)
{
    int min = 0, i = 0, j = 0, tmp = 0;;

    for (i = 0; i < size; i++)
    {
        min = i;
        for (j = i + 1; j < size; j++)
        {
            if (a[min] > a[j])
                min = j;
        }
        
        if (i != min) 
        {
            tmp = a[min];
            a[min] = a[i];
            a[i] = tmp;
        }
    }

    return 0;
}

void insert_sort(int a[], int size)
{
    int i = 0, j = 0, tmp = 0;

    for (i = 0; i < size - 1; i++)
    {
        if (a[i] > a[i + 1])
        {
            tmp = a[i + 1];
            for (j = i; j >=0 && a[j] > tmp; j--)
                a[j + 1] = a[j];
            a[j + 1] = tmp;
        }
    }

    return;
}

/*********************************************************
 ******************    Main Function    ******************
 *********************************************************/
int main(int agrc, char *agrv[])
{
    int rt = 0; /* return value of function main */
    int a[] = {1, 3, 2, 4, 5, 8, 7, 6};
    int size = sizeof(a)/sizeof(a[0]);
    int i = 0;

    printf("before:\n");
    for (i = 0; i < size; i++)
        printf("%d ", a[i]);
    printf("\n");

    //select_sort(a, size);
    insert_sort(a, size);

    printf("after:\n");
    for (i = 0; i < size; i++)
        printf("%d ", a[i]);
    printf("\n");


    return rt;
}

