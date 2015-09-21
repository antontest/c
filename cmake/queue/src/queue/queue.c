#include <queue.h>

/**
 * @brief enqueue 
 *
 * @param queue [in] queue
 * @param ele   [in] element of queue
 * 
 * @return 0, if succ; -1, if faile
 */
int enqueue(void *queue, void *ele)
{
    struct common_queue *pqueue = NULL;
    if (queue == NULL || ele == NULL) return -1;
    
    pqueue = (struct common_queue *)queue;
    if (pqueue->head == NULL) {
        pqueue->head = ele;
    } else {
        pqueue->tail->next = ele;
    }

    pqueue->tail = ele;
    pqueue->tail->next = NULL;

    return 0;
}

/**
 * @brief dequeue 
 *
 * @param queue [in] queue
 *
 * @return element of queue, if succ; -1, if failed
 */
void *dequeue(void *queue)
{
    void *ele = NULL;
    struct common_queue *pqueue = NULL;
    
    if (queue == NULL) return NULL;
    pqueue = (struct common_queue *)queue;
    ele = pqueue->head;
    if (pqueue->head == NULL) return NULL;
    pqueue->head = pqueue->head->next;

    return ele;
}

/**
 * @brief queue_destroy 
 *
 * @param queue [in] queue
 */
void queue_destroy(void *queue)
{
    void *ele = NULL;
    
    if (queue == NULL) return;
    while ((ele = dequeue(queue)) != NULL)
        free(ele);
}

/**
 * @brief jump_head 
 *
 * @param queue [in] queue
 * @param ele   [in] element of queue
 *
 * @return 
 */
int jump_head(void *queue, void *ele)
{
    struct common_queue *pqueue = NULL;
    struct element *pele = NULL;
    if (queue == NULL || ele == NULL) return -1;
    
    pqueue = (struct common_queue *)queue;
    pele = (struct element *)ele;
    pele->next = pqueue->head;
    pqueue->head = pele;

    return 0;
}

/**
 * @brief jump_queue 
 *
 * @param queue [in] queue
 * @param ele1  [in] jump front of ele1
 * @param ele2  [in] element of inserting
 *
 * @return 0, if succ; -1, if failed
 */
int jump_queue(void *queue, void *ele1, void *ele2)
{
    struct common_queue *pqueue = NULL;
    struct element *pele = NULL, *pele1 = NULL, *pele2 = NULL;
    struct element *pele_pre = NULL;
    
    if (queue == NULL || ele2 == NULL) return -1;
    if (ele1 == NULL) return jump_head(queue, ele2);

    pqueue = (struct common_queue *)queue;
    pele1 = (struct element *)ele1;
    pele2 = (struct element *)ele2;

    pele = pqueue->head;
    while (1) {
        if (pele == NULL) break;
        if (pele == pele1) break;
        pele_pre = pele;
        pele = pele->next;
    }
    
    if (pele == NULL) return -1;
    if (pele_pre == NULL) return jump_head(queue, ele2);

    pele_pre->next = pele2;
    pele2->next = pele;

    return 0;
}

/**
 * @brief del_element -- delete element from queue
 *
 * @param queue [in] queue 
 * @param ele   [in] element
 *
 * @return 0, if succ; -1, if failed
 */
int del_element(void *queue, void *ele)
{
    struct common_queue *pqueue = NULL;
    struct element *pele = NULL;
    struct element *pele_pre = NULL;
    
    if (queue == NULL || ele == NULL) return -1;

    pqueue = (struct common_queue *)queue;

    pele = pqueue->head;
    while (1) {
        if (pele == NULL) break;
        if (pele == ele) break;
        pele_pre = pele;
        pele = pele->next;
    }
    
    if (pele == NULL) return -1;
    if (pele_pre == NULL) {
        pqueue->head = pqueue->head->next;
    } else {
        pele_pre->next = pele->next;
    }
    free(ele);

    return 0;
}
