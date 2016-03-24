#ifndef __CINT_H__
#define __CINT_H__

typedef struct cint_t cint_t;
struct cint_t {
    /**
     * @brief add int number to array
     */
    int (*add) (cint_t *this, int n);

    /**
     * @brief get int number
     */
    int (*get_at) (cint_t *this, int index);
    int (*get_first) (cint_t *this);
    int (*get_last) (cint_t *this);

    /**
     * @brief insert int number to array
     */
    int (*insert) (cint_t *this, int index, int n);

    /**
     * @brief delete number in array
     */
    int (*remove_at) (cint_t *this, int index);

    /**
     * @brief remove_last number
     */
    int (*remove_last) (cint_t *this);

    /**
     * @brief clear all int number
     */
    void (*remove_all) (cint_t *this);

    /**
     * @brief clone a cint array 
     */
    cint_t *(*clone) (cint_t *this);

    /**
     * @brief destroy instance and free memory
     */
    void (*destroy) (cint_t *this);

    /**
     * @brief enumerate int array
     */
    void (*reset_enumerate) (cint_t *this);
    int (*enumerate) (cint_t *this, int *n);

    /**
     * @brief get length of int array
     */
    int (*get_length) (cint_t *this);

    /**
     * @brief get cint buffer size
     */
    int (*get_size) (cint_t *this);

    /**
     * @brief reset_size
     */
    int (*reset_size) (cint_t *this, int new_size);

    /**
     * @brief print int array
     */
    void (*print) (cint_t *this);
};

/**
 * @brief create cint_t instance
 */
cint_t *cint_create(int size);

#endif /* __CINT_H__ */
