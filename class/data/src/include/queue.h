#ifndef __QUEUE_H__
#define __QUEUE_H__

typedef struct queue_t queue_t;
struct queue_t {
    /**
     * @brief enqueue element to this queue
     */
    int (*enqueue) (queue_t *this, void *item);

    /**
     * @brief dequeue element from this queue
     */
    int (*dequeue) (queue_t *this, void **item);

    /**
     * @brief remove element from queue
     */
    int (*remove) (queue_t *this, void *item, int (*compare) (void *, void *));

    /**
     * @brief item count of this queue
     */
    int (*get_count) (queue_t *this);

    /**
     * @brief clear element and free
     */
    void (*clear) (queue_t *this);
    
    /**
     * @brief destroy instance and free memory
     *
     * @param 
     */
    void (*destroy) (queue_t *this);
};

/**
 * @brief create_queue instance
 */
queue_t *create_queue();

#endif /* __QUEUE_H__ */
