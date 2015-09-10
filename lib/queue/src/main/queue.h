#ifndef __QUEUE_H__
#define __QUEUE_H__

#include <stdlib.h>
#include <string.h>
#include <sys/sysinfo.h>
#include <stdbool.h>


typedef struct queue_node {
    unsigned int birth_seq; /* node created with a seq, started with 0 */
    unsigned int seq;       /* seq started with 0 */
    unsigned int type;      /* information of queue member */
    long create_time;       /* time of queue node created */
    long enqueue_time;      /* time of enqueue */
    long dequeue_time;      /* time of dequeue */ 
    int hold_cnt;           /* hold member info until hold_cnt equal to 0 */
    struct queue_node *next;
} queue_node_t;

typedef struct queue {
    unsigned int queue_id;  /* id of queue */
    unsigned int queue_len; /* total len of queue */
    bool stop_enqueue;      /* flag of stop enqueue */
    bool stop_dequeue;      /* flag of stop dequeue */
    int hold_total_sum;     /* hold member in this queue */
    queue_node_t *front;
    queue_node_t *curr;
    queue_node_t *rear;
} queue_t;

/**
 * @brief queue_node_create 
 *
 * @return new queue node, if succ; NULL, if failed
 */
struct queue_node * queue_node_create();

/**
 * @brief queue_init 
 *
 * @param q [in] init queue
 *
 * @return 0, if succ; -1, if failed
 */
int queue_init(struct queue *q);

/**
 * @brief queue_deinit 
 *
 * @param q [in] queue
 *
 * @return 0, if succ; -1, if failed
 */
int queue_deinit(struct queue *q);

/**
 * @brief enqueue 
 *
 * @param q    [in] queue
 * @param node [in] queue node
 *
 * @return 0, if succ; -1, if failed
 */
int enqueue(struct queue *q, struct queue_node *node);

/**
 * @brief dequeue 
 *
 * @param q [in] queue
 *
 * @return queue node of front, if succ; NULL, if failed
 */
struct queue_node * dequeue(struct queue *q);

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
        unsigned int seq);

/**
 * @brief quit_queue 
 *
 * @param q   [in] queue
 * @param seq [in] seq of quit node in queue
 *
 * @return 0, if succ; 1, if failed
 */
int quit_queue(struct queue *q, int seq);

#endif

