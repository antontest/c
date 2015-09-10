#include <stdio.h>
#include <string.h>

void BubbleSort3(int a[], int n)  
{  
    int j, k , t, count = 0;  
    int flag;  
    
    flag = n;  
    while (flag > 0)  
    {  
        k = flag;  
        flag = 0;  
        for (j = 1; j < k; j++)
        {
            if (a[j - 1] > a[j])  
            {
                t = a[j];
                a[j] = a[j - 1];
                a[j - 1] = t;
                //Swap(a[j - 1], a[j]);  
                flag = j;
                count++;
            }
        }
        printf("%d\n",count);
    }  
}  

int bubble_sort(int arr[],int size)
{
    int j = 0, t, n = size, count = 0;
    char flag = size;
    
    while(flag > 0)
    {
        n = flag;
        flag = 0;
        for(j = 1; j < n ; j++)
        {
            if(arr[j - 1] < arr[j])
            {
                t = arr[j];
                arr[j] = arr[j - 1];
                arr[j - 1] = t;
                flag = j;
                count++;
            }
            //count++;
        }
    }
    printf("count = %d.\n", count);

    return 0;
}

void print_array(int arr[], int size)
{
    int i = 0;
    for(i = 0; i < size; i++)
        printf("%d ", arr[i]);
    printf("\n");

    return;
}

int main(int agrc, char *argv[])
{
    int arr[] = {5,7,8,5,4,3,1,2};
    int size = sizeof(arr) / sizeof(int);
    
    print_array(arr, size);
    bubble_sort(arr, size);
    //BubbleSort3(arr, size);
    print_array(arr, size);
    
    return 0;
}
