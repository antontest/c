#include <queue.h>

/**
 * @brief enq 
 *
 * @param q [in] q
 * @param ele   [in] element of q
 * 
 * @return 0, if succ; -1, if faile
 */
int enqueue(void *q, void *ele)
{
    struct common_queue *queue = NULL;
    if (q == NULL || ele == NULL) return -1;
    
    queue = (struct common_queue *)q;
    if (queue->head == NULL) {
        queue->head = ele;
    } else {
        queue->tail->next = ele;
    }

    queue->tail = ele;
    queue->tail->next = NULL;

    return 0;
}

/**
 * @brief deq 
 *
 * @param q [in] q
 *
 * @return element of q, if succ; -1, if failed
 */
void *dequeue(void *q)
{
    void *ele = NULL;
    struct common_queue *queue = NULL;
    
    if (q == NULL) return NULL;
    queue = (struct common_queue *)q;
    ele = queue->head;
    if (ele != NULL)
        queue->head = queue->head->next;

    return ele;
}

/**
 * @brief q_destroy 
 *
 * @param q [in] q
 */
void queue_destroy(void *q)
{
    void *ele = NULL;
    
    if (q == NULL) return;
    while ((ele = dequeue(q)) != NULL)
        free(ele);
}

/**
 * @brief jump_head 
 *
 * @param q [in] q
 * @param ele   [in] element of q
 *
 * @return 
 */
int jump_head(void *q, void *ele)
{
    if (q == NULL || ele == NULL) return -1;
    
    ((struct element *)ele)->next = ((struct common_queue *)q)->head;
    ((struct common_queue *)q)->head = ele;

    return 0;
}

/**
 * @brief jump_q 
 *
 * @param q [in] q
 * @param ele1  [in] jump front of ele1
 * @param ele2  [in] element of inserting
 *
 * @return 0, if succ; -1, if failed
 */
int jump_queue(void *q, void *ele1, void *ele2)
{
    struct common_queue queue = {0};;
    struct element *ele = NULL;
    
    if (q == NULL || ele2 == NULL) return -1;
    if (ele1 == NULL) return jump_head(q, ele2);

    /**
     * if found ele1, then insert ele2 before ele1
     */
    while ((ele = dequeue(q)) != NULL) {
        if (ele == ele1) {
            enqueue(&queue, ele2);
            enqueue(&queue, ele);
            break;
        } else enqueue(&queue, ele);
    }

    /**
     * if found ele1, then merge queue
     */
    if (ele != NULL) {
        queue.tail->next = ((struct common_queue *)q)->head;
        queue.tail = ((struct common_queue *)q)->tail;
    }
    ((struct common_queue *)q)->head = queue.head;

    return 0;
}

/**
 * @brief del_element -- delete element from q
 *
 * @param q [in] q 
 * @param ele   [in] element
 *
 * @return 0, if succ; -1, if failed
 */
int del_element(void *q, void *ele)
{
    struct common_queue *queue = NULL;
    struct element *pele = NULL;
    struct element *pele_pre = NULL;
    
    if (q == NULL || ele == NULL) return -1;

    queue = (struct common_queue *)q;

    pele = queue->head;
    while (1) {
        if (pele == NULL) break;
        if (pele == ele) break;
        pele_pre = pele;
        pele = pele->next;
    }
    
    if (pele == NULL) return -1;
    if (pele_pre == NULL) {
        queue->head = queue->head->next;
    } else {
        pele_pre->next = pele->next;
    }
    free(ele);

    return 0;
}

/**
 * @brief get_head 
 *
 * @param q
 *
 * @return head element, if succ; NULL, if failed
 */
void * get_head(void *q)
{
    if (q == NULL) return NULL;

    return ((struct common_queue *)q)->head;
}

/**
 * @brief get_queue_length 
 *
 * @param q [in] queue
 *
 * @return len of queue, if succ; -1, if q is null
 */
int get_queue_length(void *q)
{
    struct element *ele = NULL;
    int len = 0;

    if (q == NULL) return -1;
    ele = ((struct common_queue *)q)->head;
    while (ele != NULL) {
        len++;
        ele = ele->next;
    }

    return len;
}

/**
 * @brief exchange 
 *
 * @param q     [in] queue
 * @param ele1  [in] element1
 * @param ele2  [in] element2
 *
 * @return 0, if succ; -1, if failed
 */
int exchange(void *q, void *ele1, void *ele2)
{
    struct element *pele = NULL, *ele = NULL;;
    struct common_queue queue_before = {0}, queue_after = {0};

    /**
     * 1. if q, ele1, ele2 has one be null, then return
     * 2. if ele1 == ele2, then return
     */
    if (q == NULL || ele1 == NULL || ele2 == NULL) return -1;
    if (ele1 == ele2) return 0;

    /**
     * find the first ele, and save it
     * if could not find , then return
     */
    while ((pele = dequeue(q)) != NULL) {
        if (pele == ele1 || pele == ele2) {
            ele = pele;
            break;
        } else enqueue(&queue_before, pele);
    }
    if (pele == NULL) goto over;

    /**
     * if found the second ele, then break and deal the queue
     * if could not find , then return
     */
    while ((pele = dequeue(q)) != NULL) {
        if (pele == ele1 || pele == ele2) {
            enqueue(&queue_before, pele);
            enqueue(&queue_after, ele);
            break;
        } else enqueue(&queue_after, pele);
    }
    if (pele == NULL)
        enqueue(&queue_before, ele);

    /**
     * merge queue before, after and q
     */
    queue_before.tail->next = queue_after.head;
    queue_before.tail = queue_after.tail;
    queue_before.tail->next = ((struct common_queue *)q)->head;
    queue_before.tail = ((struct common_queue *)q)->tail;
over:
    ((struct common_queue *)q)->head = queue_before.head;
    
    return 0;
}

/**
 * @brief reverse_queue 
 *
 * @param q [in] queue
 *
 * @return 0, if succ; -1, if failed
 */
int reverse_queue(void *q)
{
    struct element *ele = NULL, *pre = NULL, *cur = NULL;

    if (q == NULL) return -1;

    /**
     * deal tail node
     */
    ele = ((struct common_queue *)q)->head;
    ((struct common_queue *)q)->tail = ele;
    
    /**
     * reverse
     */
    while (ele != NULL) {
        pre = cur;
        cur = ele;
        ele = ele->next;
        cur->next = pre;
    }
    ((struct common_queue *)q)->head = cur;
    ((struct common_queue *)q)->tail->next = NULL;

    return 0;
}

/**
 * @brief bubble_queue 
 *
 * @param q   [in] queue
 * @param cmp [in] callback
 */
void bubble_queue(void *q, int (*cmp)(const void *, const void *))
{
    struct element *pre = NULL, *cur = NULL, *next= NULL;
    struct common_queue *queue = (struct common_queue *)q; 
    int exchange_flag = 0;

    if (q == NULL || cmp == NULL) return;
    cur = ((struct common_queue *)q)->head;
    if (cur == NULL) return;
    next = cur->next;

    while (1) {
        /**
         * reset pointer
         */
        pre = NULL;
        cur = queue->head;
        next = cur->next;
        exchange_flag = 0;

        while (next != NULL) {
            /**
             * exchange two elements
             */
            if (cmp(cur, next) > 0) {
                if (pre != NULL)
                    pre->next = next;
                else queue->head = next;
                cur->next = next->next;
                next->next = cur;

                /**
                 * set exchange flag
                 */
                exchange_flag = 1;
            }

            /**
             * next compare
             */
            pre = cur;
            cur = next;
            next = next->next;
        }
        if (!exchange_flag) break;
    }
    queue->tail = cur;
}
