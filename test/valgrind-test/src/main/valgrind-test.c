/**
 * usual head files
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*********************************************************
 **************    Function Declaration    ***************
 *********************************************************/
int* func(void)  
{  
    int* x = malloc(10 * sizeof(int));
    x[10] = 0;  //问题1: 数组下标越界
    return x;
}    

/*********************************************************
 ******************    Main Function    ******************
 *********************************************************/
int main(int agrc, char *agrv[])
{
    int rt = 0; /* return value of function main */
    int* x = NULL;  
    x = func();  
    free(x);    
    x = NULL; 
 
    return rt;
}
