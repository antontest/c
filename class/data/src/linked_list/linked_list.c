#include <stdlib.h>
#include <stdarg.h>
#include <utils/utils.h>
#include <linked_list.h>

typedef struct element_t element_t;

/**
 * This element holds a pointer to the value it represents.
 */
struct element_t {

	/**
	 * Value of a list item.
	 */
	void *value;

	/**
	 * Previous list element.
	 *
	 * NULL if first element in list.
	 */
	element_t *previous;

	/**
	 * Next list element.
	 *
	 * NULL if last element in list.
	 */
	element_t *next;
};

/**
 * Creates an empty linked list object.
 */
element_t *element_create(void *value)
{
	element_t *this;
	INIT(this,
		.value = value,
	);
	return this;
}


typedef struct private_linked_list_t private_linked_list_t;

/**
 * Private data of a linked_list_t object.
 *
 */
struct private_linked_list_t {
	/**
	 * Public part of linked list.
	 */
	linked_list_t public;

	/**
	 * Number of items in the list.
	 */
	int count;

	/**
	 * First element in list.
	 * NULL if no elements in list.
	 */
	element_t *first;

	/**
	 * Current element in list.
	 * NULL if no elements in list.
	 */
	element_t *current;

	/**
	 * Last element in list.
	 * NULL if no elements in list.
	 */
	element_t *last;
};


METHOD(linked_list_t, get_count, int,
	private_linked_list_t *this)
{
	return this->count;
}

METHOD(linked_list_t, insert_first, void,
	private_linked_list_t *this, void *item)
{
	element_t *element;

	element = element_create(item);
	if (this->count == 0)
	{
		/* first entry in list */
		this->first = element;
		this->last = element;
	}
	else
	{
		element->next = this->first;
		this->first->previous = element;
		this->first = element;
	}
	this->count++;
}

/**
 * unlink an element form the list, returns following element
 */
static element_t* remove_element(private_linked_list_t *this,
								 element_t *element)
{
	element_t *next, *previous;

	next = element->next;
	previous = element->previous;
	free(element);
	if (next)
	{
		next->previous = previous;
	}
	else
	{
		this->last = previous;
	}
	if (previous)
	{
		previous->next = next;
	}
	else
	{
		this->first = next;
	}
	if (--this->count == 0)
	{
		this->first = NULL;
		this->last = NULL;
	}
	return next;
}

METHOD(linked_list_t, get_first, status_t,
	private_linked_list_t *this, void **item)
{
	if (this->count == 0)
	{
		return NOT_FOUND;
	}
	*item = this->first->value;
	return SUCCESS;
}

METHOD(linked_list_t, reset_current, status_t,
	private_linked_list_t *this)
{
	if (this->count == 0)
		return NOT_FOUND;
	this->current = this->first;
	return SUCCESS;
}

METHOD(linked_list_t, get_next, status_t,
	private_linked_list_t *this, void **item)
{
	if (this->count == 0)
		return NOT_FOUND;
	if (!this->current) this->current = this->first;
	*item = this->current->value;
	this->current = this->current->next;
	return SUCCESS;
}

METHOD(linked_list_t, remove_first, status_t,
	private_linked_list_t *this, void **item)
{
	if (get_first(this, item) == SUCCESS)
	{
		remove_element(this, this->first);
		return SUCCESS;
	}
	return NOT_FOUND;
}

METHOD(linked_list_t, insert_last, void,
	private_linked_list_t *this, void *item)
{
	element_t *element;

	element = element_create(item);
	if (this->count == 0)
	{
		/* first entry in list */
		this->first = element;
		this->last = element;
	}
	else
	{
		element->previous = this->last;
		this->last->next = element;
		this->last = element;
	}
	this->count++;
}

METHOD(linked_list_t, get_last, status_t,
	private_linked_list_t *this, void **item)
{
	if (this->count == 0)
	{
		return NOT_FOUND;
	}
	*item = this->last->value;
	return SUCCESS;
}

METHOD(linked_list_t, remove_last, status_t,
	private_linked_list_t *this, void **item)
{
	if (get_last(this, item) == SUCCESS)
	{
		remove_element(this, this->last);
		return SUCCESS;
	}
	return NOT_FOUND;
}

METHOD(linked_list_t, remove_, int,
	private_linked_list_t *this, void *item, bool (*compare)(void*,void*))
{
	element_t *current = this->first;
	int removed = 0;

	while (current)
	{
		if ((compare && compare(current->value, item)) ||
			(!compare && current->value == item))
		{
			removed++;
			current = remove_element(this, current);
		}
		else
		{
			current = current->next;
		}
	}
	return removed;
}

/*
METHOD(linked_list_t, find_first, status_t,
	private_linked_list_t *this, linked_list_match_t match,
	void **item, void *d1, void *d2, void *d3, void *d4, void *d5)
{
	element_t *current = this->first;

	while (current)
	{
		if ((match && match(current->value, d1, d2, d3, d4, d5)) ||
			(!match && item && current->value == *item))
		{
			if (item != NULL)
			{
				*item = current->value;
			}
			return SUCCESS;
		}
		current = current->next;
	}
	return NOT_FOUND;
}
*/

METHOD(linked_list_t, find_first, status_t,
	private_linked_list_t *this, 
	void **item, void *key, int (*cmp) (void *, void *))
{
	element_t *current = this->first;
    if (!cmp || !key || !item) return NOT_FOUND;

	while (current)
	{
		if (cmp && !cmp(current->value, key))
		{
			if (item != NULL)
			{
				*item = current->value;
			}
			return SUCCESS;
		}
		current = current->next;
	}
	return NOT_FOUND;
}

METHOD(linked_list_t, invoke_offset, void,
	private_linked_list_t *this, size_t offset,
	void *d1, void *d2, void *d3, void *d4, void *d5)
{
	element_t *current = this->first;
	linked_list_invoke_t *method;

	while (current)
	{
		method = current->value + offset;
		(*method)(current->value, d1, d2, d3, d4, d5);
		current = current->next;
	}
}

METHOD(linked_list_t, invoke_function, void,
	private_linked_list_t *this, linked_list_invoke_t fn,
	void *d1, void *d2, void *d3, void *d4, void *d5)
{
	element_t *current = this->first;

	while (current)
	{
		fn(current->value, d1, d2, d3, d4, d5);
		current = current->next;
	}
}

METHOD(linked_list_t, clone_offset, linked_list_t*,
	private_linked_list_t *this, size_t offset)
{
	element_t *current = this->first;
	linked_list_t *clone;

	clone = linked_list_create();
	while (current)
	{
		void* (**method)(void*) = current->value + offset;
		clone->insert_last(clone, (*method)(current->value));
		current = current->next;
	}

	return clone;
}

METHOD(linked_list_t, clear_, void, private_linked_list_t *this)
{
    void *element = NULL;
    int cnt = this->count;

    while (cnt-- > 0) {
        _remove_first(this, &element);
        if (element) free(element);
        element = NULL;
    }
}

METHOD(linked_list_t, destroy, void,
	private_linked_list_t *this)
{
	void *value;

	/* Remove all list items before destroying list */
	while (remove_first(this, &value) == SUCCESS)
	{
		/* values are not destroyed so memory leaks are possible
		 * if list is not empty when deleting */
	}
	clear_(this);
	free(this);
}

METHOD(linked_list_t, destroy_offset, void,
	private_linked_list_t *this, size_t offset)
{
	element_t *current = this->first, *next;

	while (current)
	{
		void (**method)(void*) = current->value + offset;
		(*method)(current->value);
		next = current->next;
		free(current);
		current = next;
	}
	free(this);
}

METHOD(linked_list_t, destroy_function, void,
	private_linked_list_t *this, void (*fn)(void*))
{
	element_t *current = this->first, *next;

	while (current)
	{
		fn(current->value);
		next = current->next;
		free(current);
		current = next;
	}
	free(this);
}

/*
 * Described in header.
 */
linked_list_t *linked_list_create()
{
	private_linked_list_t *this;

	INIT(this,
		.public = {
			.get_count = _get_count,
			.get_first = _get_first,
			.get_next  = _get_next,
			.get_last  = _get_last,
			.reset_current = _reset_current,
			.find_first   = (void*)_find_first,
			.insert_first = _insert_first,
			.insert_last  = _insert_last,
			.remove_first = _remove_first,
			.remove_last  = _remove_last,
			.remove          = _remove_,
			.invoke_offset   = (void*)_invoke_offset,
			.invoke_function = (void*)_invoke_function,
			.clone_offset     = _clone_offset,
			.clear            = _clear_,
			.destroy          = _destroy,
			.destroy_offset   = _destroy_offset,
			.destroy_function = _destroy_function,
		},
		.first   = NULL,
		.current = NULL,
		.last    = NULL,
	);

	return &this->public;
}


/*
 * See header.
 */
linked_list_t *linked_list_create_with_items(void *item, ...)
{
	linked_list_t *list;
	va_list args;

	list = linked_list_create();

	va_start(args, item);
	while (item)
	{
		list->insert_last(list, item);
		item = va_arg(args, void*);
	}
	va_end(args);

	return list;
}


