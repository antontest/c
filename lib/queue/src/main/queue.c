#include "queue.h"
#include <stdio.h>

static unsigned int queue_id = 1;
static unsigned int queue_seq = 1;

typedef struct mem_info {
    unsigned int type;    
} mem_info_t;

static long get_systime()
{
    struct sysinfo uptime = {0};

    sysinfo(&uptime);
    
    return uptime.uptime;
}

/**
 * @brief queue_node_create 
 *
 * @return new queue node, if succ; NULL, if failed
 */
struct queue_node * queue_node_create()
{
    struct queue_node *node = NULL;

    if ((node = (struct queue_node *)malloc\
                (sizeof(struct queue_node))) == NULL)
        return NULL;

    memset(node, 0, sizeof(struct queue_node));
    node->birth_seq = queue_seq++;
    node->create_time = get_systime();

    return node;
}

/**
 * @brief queue_init 
 *
 * @param q [in] init queue
 *
 * @return 0, if succ; -1, if failed
 */
int queue_init(struct queue *q)
{
    if (q == NULL) return -1;

    q->queue_id = queue_id++;
    q->queue_len = 0;
    q->front = NULL;
    q->rear = NULL;

    return 0;
}

/**
 * @brief queue_deinit 
 *
 * @param q [in] queue
 *
 * @return 0, if succ; -1, if failed
 */
int queue_deinit(struct queue *q)
{
    queue_node_t *node = NULL;

    if (q == NULL) return -1;
    while (q->front)
    {
        node = q->front;
        q->front = q->front->next;
        free(node);
    }

    free(q);
    return 0;
}

/**
 * @brief enqueue 
 *
 * @param q    [in] queue
 * @param node [in] queue node
 *
 * @return 0, if succ; -1, if failed
 */
int enqueue(struct queue *q, struct queue_node *node)
{
    if (q == NULL) return -1;
    if (q->front == NULL) 
    {
        q->front = node;
        q->curr = node;
    }
    else q->rear->next = node;

    node->seq = ++q->queue_len;
    q->rear = node; 

    return 0;
}

/**
 * @brief dequeue 
 *
 * @param q [in] queue
 *
 * @return queue node of front, if succ; NULL, if failed
 */
struct queue_node * dequeue(struct queue *q)
{
    
    struct queue_node *node = NULL;
    struct queue_node *fix_node = NULL;
    int seq = 1;
    if (q == NULL) return NULL;

    node =  q->curr;
    if (node == NULL) return node;
    q->curr = q->curr->next;
    if (q->curr == NULL && q->front != NULL)
        q->curr = q->front;
    if (!node->hold_cnt)
        q->front = q->curr;
    q->queue_len--;

    fix_node = q->front;
    while (fix_node != NULL)
    {
        fix_node->seq = seq++;
        fix_node = fix_node->next;
    }

    return node;
}

/**
 * @brief jump_queue
 *
 * @param q    [in] queue
 * @param node [in] new queue node
 * @param seq  [in] jump seq
 *
 * @return 0, if succ; -1, if failed
 */
int jump_queue(struct queue *q, struct queue_node *node, \
        unsigned int seq)
{
    struct queue_node *n = NULL;
    struct queue_node *pre = NULL;

    if (q == NULL || seq <=0) return -1;
    
    n = q->front;
    while (n != NULL)
    {
        if (n->seq == seq)
            break;
        pre = n;
        n = n->next;
    }

    node->next = n;
    if (pre == NULL)
    {
        if (q->front == q->curr)
            q->curr = node;
        q->front = node;
    }
    else 
        pre->next = node;
    if (seq > q->queue_len)
        seq = q->queue_len;
    node->seq = seq;


    n = node->next;
    while (n != NULL)
    {
        n->seq = ++seq;
        n = n->next;
    }

    return 0;
}

/**
 * @brief quit_queue 
 *
 * @param q   [in] queue
 * @param seq [in] seq of quit node in queue
 *
 * @return 0, if succ; 1, if failed
 */
int quit_queue(struct queue *q, int seq)
{
    struct queue_node *node = NULL;
    struct queue_node *pre = NULL;

    if (q == NULL || seq <= 0) 
        return -1;

    node = q->front;
    while (node != NULL)
    {
        if (node->seq == seq) 
            break;

        pre = node;
        node = node->next;
    }

    if (node == NULL) return -1;

    if (node == q->curr) q->curr = q->curr->next;
    if (pre == NULL)
        q->front = q->front->next;
    else if (node->seq == seq)
        pre->next = node->next;
    free(node);

    node = pre->next;
    while (node != NULL)
    {
        node->seq = seq++;
        node = node->next;
    }

    return 0;
}

void print_queue(struct queue *q)
{
    struct queue_node *node = NULL;

    if (q == NULL) return ;
    
    node = q->front;
    while (node != NULL)
    {
        printf("node seq: %u, birth seq: %u\n", node->seq, \
                node->birth_seq);
        node = node->next;
    }

    return;
}

/*
int main(int agrc, char *agrv[])
{
    struct queue *q = NULL;
    struct queue_node *node = NULL;

    if ((q = (struct queue *)malloc(sizeof(struct queue))) == NULL)
        return -1;

    memset(q, 0, sizeof(struct queue));
    queue_init(q);

    node = queue_node_create();
    enqueue(q, node);
    node = queue_node_create();
    enqueue(q, node);
    node = queue_node_create();
    enqueue(q, node);
    node = queue_node_create();
    jump_queue(q, node, 3);
    //node = dequeue(q);
    //if (node != NULL) free(node);
    quit_queue(q, 2);

    print_queue(q);
    queue_deinit(q);

    return 0;
}
*/
