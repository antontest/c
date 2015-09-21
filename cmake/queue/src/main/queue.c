/**
 * usual head files
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "queue.h"

/*********************************************************
 **************    Function Declaration    ***************
 *********************************************************/
struct elem {
    struct elem *next;
    int id;
};
struct queue {
    struct elem *head;
    struct elem *tail;
};

/*********************************************************
 ******************    Main Function    ******************
 *********************************************************/
int main(int agrc, char *agrv[])
{
    int rt = 0; /* return value of function main */

    struct queue q = {0};
    struct elem *ele = NULL, *ele1 = NULL, *ele2 = NULL;
    
    ele = (struct elem *)malloc(sizeof(struct elem));
    ele->id = 1;
    enqueue(&q, ele); 

    ele2 = (struct elem *)malloc(sizeof(struct elem));
    ele2->id = 2;
    enqueue(&q, ele2); 

    ele = (struct elem *)malloc(sizeof(struct elem));
    ele->id = 3;
    enqueue(&q, ele); 

    ele = (struct elem *)malloc(sizeof(struct elem));
    ele->id = 4;
    jump_head(&q, ele); 

    ele1 = (struct elem *)malloc(sizeof(struct elem));
    ele1->id = 5;
    jump_queue(&q, ele2, ele1); 

    del_element(&q, ele2);
    del_element(&q, q.head);

    while ((ele = dequeue(&q)) != NULL) {
        printf("id: %d\n", ele->id);
        free(ele);
    }

    return rt;
}
