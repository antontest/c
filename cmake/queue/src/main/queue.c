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

int cmp(const void *ele1, const void* ele2)
{
    return ((struct elem *)ele1)->id - ((struct elem *)ele2)->id;
}

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
    //enqueue(&q, ele2); 
    jump_head(&q, ele2); 

    ele = (struct elem *)malloc(sizeof(struct elem));
    ele->id = 3;
    //enqueue(&q, ele); 
    jump_head(&q, ele); 

    ele = (struct elem *)malloc(sizeof(struct elem));
    ele->id = 4;
    enqueue(&q, ele); 
    //jump_head(&q, ele); 

    ele1 = (struct elem *)malloc(sizeof(struct elem));
    ele1->id = 5;
    enqueue(&q, ele1); 
    //jump_head(&q, ele1); 
    //jump_queue(&q, ele2, ele1); 

    //del_element(&q, ele2);
    //del_element(&q, q.head);

    ele = get_head(&q);
    printf("head id: %d\n", ele->id);
    printf("queue size: %d\n", get_queue_length(&q));

    //reverse_queue(&q);
    printf("tail id: %d\n", q.tail->id);

    bubble_queue(&q, cmp);
    printf("bubble tail id: %d\n", q.tail->id);
    //struct element *ele3 = (struct element *)malloc(sizeof(struct element));
    //exchange(&q, q.head, q.head->next);
    //exchange(&q, q.head, q.tail);
    while ((ele = dequeue(&q)) != NULL) {
        printf("id: %d\n", ele->id);
        free(ele);
    }
    //free(ele3);

    return rt;
}
