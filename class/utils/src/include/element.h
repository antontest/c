#ifndef __ELEMENT_H__
#define __ELEMENT_H__

typedef struct element_t element_t;
struct element_t {
    void *value;
    element_t *previous;
    element_t *next;
};

typedef struct list_t list_t;
struct list_t {
    element_t *first;
    element_t *last;
};

element_t *create_element(void *value);
element_t *remove_element(list_t *list, element_t *element);

#endif /* __ELEMENT_H__ */
