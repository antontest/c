#ifndef __POOL_H__
#define __POOL_H__

typedef struct pool_t pool_t;
struct pool_t {
    /**
     * @brief add task to thread pool
     * @param work  task 
     * @param arg   parameter of task
     */
    int (*add) (pool_t *this, void (*work) (void *), void *arg);

    /**
     * @brief destroy instance and free memory 
     */
    void (*destroy) (pool_t *this);
};

/**
 * @brief create pool instance 
 * @param size   pool thread size
 */
pool_t *create_pool(int size);

#endif /* __POOL_H__ */
