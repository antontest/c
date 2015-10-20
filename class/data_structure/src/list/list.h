#ifndef __LIST_H__
#define __LIST_H__

typedef struct list_t list_t;

struct list_t {
    /**
     * @brief get items in the list.
     *
     * @return number of items in list
     */
    int (*get_count) (list_t *this);

    /**
     * @brief Inserts a new item.
     *
     * @param item item value to insert in list
     */
    void (*insert) (list_t *this, void *item);
    void (*delete) (list_t *this, void *item);

    /**
     * @brief Remove items from the list matching the given item.
     *
     * @param item    item to remove/pass to comparator
     * @param compare compare function, or NULL
     * @return          number of removed items
     */
    int (*remove) (list_t *this, int (*compare)(void *, void *));

    /**
     * @brief destroy a list object 
     */
    void (*destroy) (list_t *this);
};

#endif /* __LIST_H__ */
