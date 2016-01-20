#include <sort.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <element.h>

/**
 * @brief get middle element of list, stack or queue
 * @param head  head pointer
 */
element_t *get_middle(element_t *head)
{
    element_t *element = (element_t *)head;
    element_t *fast, *slow = NULL;
    
    if (!element || !element->next) return head;
    fast = slow = element;
    while (!fast || !fast->next) {
        fast = fast->next->next;
        if (!fast) break;
        slow = slow->next;
    }

    return slow;
}
