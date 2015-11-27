#include <queue.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <utils/utils.h>

typedef struct element_t element_t;
struct element_t {
    /**
     * @brief value of this queue item
     */
    void *value;

    /**
     * @brief previous item
     */
    element_t *previous;

    /**
     * @brief next item
     */
    element_t *next;
};

/**
 * @brief create element 
 */
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

typedef struct private_queue_t private_queue_t;
struct private_queue_t {
    /**
     * @brief public interface
     */
    queue_t public;

    /**
     * @brief item numbers in this queue
     */
    int count;

    /**
     * @brief front item in this queue
     */
    element_t *front;

    /**
     * @brief tail item in this queue
     */
    element_t *tail;
};

METHOD(queue_t, enqueue_, int, private_queue_t *this, void *item)
{
    element_t *element = NULL;

    element = create_element(item);
    if (!element) return -1;
    if (this->count == 0) {
        this->front = element;
        this->tail  = element;
    } else {
        element->previous = this->tail;
        this->tail->next  = element;
        this->tail        = element;
    }
    this->count++;

    return 0;
}

static element_t *remove_element(private_queue_t *this, element_t *element)
{
    element_t *previous, *next;
    previous = element->previous;
    next     = element->next;
    free(element);

    if (next) next->previous = previous;
    else this->tail = previous;

    if (previous) previous->next = next;
    else this->front = next;

    if (--this->count == 0) {
        this->front = NULL;
        this->tail  = NULL;
    }

    return next;
}

METHOD(queue_t, dequeue_, int, private_queue_t *this, void **item)
{
    element_t *element = NULL;

    if (this->count < 1) return -1;

    element = this->front;
    *item = element->value;
    remove_element(this, element);
    return 0;
}

METHOD(queue_t, remove_, int, private_queue_t *this, void *item, int (*compare) (void *, void *))
{
    element_t *current = this->front;
    int removed = 0;

    while (current) {
        if ((compare && compare(current->value, item)) || 
            (!compare && current->value == item)) {
            removed++;
            current = remove_element(this, current);
        } else {
            current = current->next;
        }
    }

    return removed;
}

METHOD(queue_t, get_count_, int, private_queue_t *this)
{
    return this->count;
}

METHOD(queue_t, clear_, void, private_queue_t *this)
{
    void *element = NULL;
    while (this->count-- > 0) {
        _dequeue_(this, &element);
        free(element);
    }
}

METHOD(queue_t, destroy_, void, private_queue_t *this)
{
    clear_(this);
    free(this);
}

queue_t *create_queue()
{
    private_queue_t *this;

    INIT(this, 
        .public = {
            .enqueue   = _enqueue_,
            .dequeue   = _dequeue_,
            .remove    = _remove_,
            .get_count = _get_count_,
            .clear     = _clear_,
            .destroy   = _destroy_,
        },
        .count = 0,
        .front = NULL,
        .tail  = NULL,
    );

    return &this->public;
}
