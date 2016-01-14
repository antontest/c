#include <element.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <utils/utils.h>

element_t *create_element(void *value)
{
    element_t *this;

    INIT(this, 
        .value    = value,
        .previous = NULL,
        .next     = NULL,
    );

    return this;
}

