#include <stack.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../../../utils/src/include/utils.h"

typedef struct element_t element_t;
struct element_t {
    /**
     * @brief value of a stack item
     */
    void *value;

    /**
     * @brief previous item in stack
     */
    element_t *previous;

    /**
     * @brief next item in stack
     */
    element_t *next;
};

element_t *create_element(void *value)
{
    element_t *this;

    INIT(this,
        .value = value,
        .previous = NULL,
        .next = NULL,
    );

    return this;
}

typedef struct private_stack_t private_stack_t;
struct private_stack_t {
    /**
     * @brief public interface
     */
    stack_t public;

    /**
     * @brief number of element in the stack
     */
    unsigned int count;
    
    /**
     * @brief top item in stack
     */
    element_t *top;
};

METHOD(stack_t, push_, int, private_stack_t *this, void *item)
{
    element_t *element = create_element(item);

    if (!this->count) {
        element->next = NULL;
        element->previous = NULL;
        this->top = element;
    } else {
        element->next = this->top;
        element->previous = NULL;
        this->top->previous = element;
        this->top = element;
    }
    this->count++;

    return 0;
}

METHOD(stack_t, pull_, int, private_stack_t *this, void **item)
{
    if (!this->count) return -1;

    *item = this->top->value;
    this->top = this->top->next;
    this->count--;

    return 0;
}

static element_t *remove_element(private_stack_t *this, element_t *element)
{
    element_t *next, *previous;

    next = element->next;
    previous = element->previous;

    if (next) {
        next->previous = previous;
    }
    if (previous) {
        previous->next = next;
    } else {
        this->top = next;
    }
    if (--this->count == 0) {
        this->top = NULL;
    }

    return next;
}

METHOD(stack_t, remove_, int, private_stack_t *this, void *item, int (*compare) (void *, void *))
{
    element_t *element = this->top;
    int removed = 0;

    while (element) {
        if ((compare && compare(element, item)) || 
            (!compare && element->value == item)) {
            element = remove_element(this, element);
        } else {
            element = element->next;
        }
    }

    return removed;
}

METHOD(stack_t, clear_, void, private_stack_t *this)
{
    void *element = NULL;

    while (this->count-- > 0) {
        _pull_(this, &element);
        if (element) free(element);
        element = NULL;
    }
}

METHOD(stack_t, destroy_, void, private_stack_t *this)
{
    clear_(this);
    free(this);
}

METHOD(stack_t, get_count_, int, private_stack_t *this)
{
    return this->count;
}

stack_t *create_stack()
{
    private_stack_t *this;

    INIT(this,
        .public = {
            .push = _push_,
            .pull = _pull_,
            .remove  = _remove_,
            .clear   = _clear_,
            .destroy = _destroy_,

            .get_count = _get_count_,
        },
        .count = 0,
        .top   = NULL,
    );

    return &this->public;
}
