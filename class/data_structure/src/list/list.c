#include "list.h"
#include <utils/utils.h>
#include <stdlib.h>

typedef struct element_t element_t;

struct element_t {
    /**
     * value of a list item
     */
    void *value;

    /**
     * list element
     */
    element_t *next;
}; 

element_t* element_create(void *value)
{
    element_t *this;
    INIT(this,
        .value = value,
    );

    return this;
}

typedef struct private_list_t private_list_t;

struct private_list_t {
    /**
     * public part of list
     */
    list_t public;

    /**
     * number of items in the list
     */
    int count;

    /**
     * element in list
     */
    private_list_t *next;
};

METHOD(list_t, get_count, int, private_list_t *this)
{
    int count = 0;
    private_list_t *ele = NULL;

    if (this == NULL) {
        goto over;
    }

    ele = this;
    while (ele != NULL) {
        count++;
        ele = ele->next;
    }

over:
    return count;
}
