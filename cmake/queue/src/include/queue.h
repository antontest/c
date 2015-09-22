#ifndef __QUEUE_H__
#define __QUEUE_H__

/**
 * usual head files
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


/******************************************************
 ********************* Struct *************************
 ******************************************************/
typedef struct element {
    struct element *next;
} element, *pelement;

typedef struct common_queue {
    struct element *head;
    struct element *tail;
} queue_t;



/******************************************************
 ****************** Queue Function ********************
 ******************************************************/
/**
 * @brief enqueue 
 *
 * @param queue [in] queue
 * @param ele   [in] element of queue
 * 
 * @return 0, if succ; -1, if faile
 */
int enqueue(void *queue, void *ele);

/**
 * @brief dequeue 
 *
 * @param queue [in] queue
 *
 * @return element of queue, if succ; -1, if failed
 */
void *dequeue(void *queue);

/**
 * @brief queue_destroy 
 *
 * @param queue [in] queue
 */
void queue_destroy(void *queue);

/**
 * @brief jump_head 
 *
 * @param queue [in] queue
 * @param ele   [in] element of queue
 *
 * @return 
 */
int jump_head(void *queue, void *ele);

/**
 * @brief jump_queue 
 *
 * @param queue [in] queue
 * @param ele1  [in] jump front of ele1
 * @param ele2  [in] element of inserting
 *
 * @return 0, if succ; -1, if failed
 */
int jump_queue(void *queue, void *ele1, void *ele2);

/**
 * @brief del_element -- delete element from queue
 *
 * @param queue [in] queue 
 * @param ele   [in] element
 *
 * @return 0, if succ; -1, if failed
 */
int del_element(void *queue, void *ele);

/**
 * @brief get_head 
 *
 * @param q
 *
 * @return head element, if succ; NULL, if failed
 */
void * get_head(void *q);

/**
 * @brief get_queue_length 
 *
 * @param q [in] queue
 *
 * @return len of queue, if succ; -1, if q is null
 */
int get_queue_length(void *q);

/**
 * @brief exchange 
 *
 * @param q     [in] queue
 * @param ele1  [in] element1
 * @param ele2  [in] element2
 *
 * @return 0, if succ; -1, if failed
 */
int exchange(void *q, void *ele1, void *ele2);

#endif
