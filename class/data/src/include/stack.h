#ifndef __STACK_H__
#define __STACK_H__

typedef struct stack_t stack_t;
struct stack_t {
    /**
     * @brief push into stack
     * @param item data
     */
    int (*push) (stack_t *this, void* item);

    /**
     * @brief top from stack
     * @param item data
     */
    int (*pull) (stack_t *this, void** item);

    /**
     * @brief  remove element
     * @return count of removed elements 
     */
    int (*remove) (stack_t *this, void* item, int (*compare) (void *, void *));

    /**
     * @brief destroy instance and free memory
     */
    void (*destroy) (stack_t *this);

    /**
     * @brief item count of stack
     */
    int (*get_count) (stack_t *this);
};

/**
 * @brief create stack instance 
 */
stack_t *create_stack();

#endif /* __STACK_H__ */
