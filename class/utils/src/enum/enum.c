#include <stddef.h>
#include <string.h>
#include "enum.h"

/**
 * See header.
 */
char *enum_to_name(enum_name_t *e, int val)
{
	do
	{
		if (val >= e->first && val <= e->last)
		{
			return e->names[val - e->first];
		}
	}
	while ((e = e->next));
	return NULL;
}

/**
 * See header.
 */
int enum_from_name(enum_name_t *e, char *name)
{
	do
	{
		int i, count = e->last - e->first + 1;

		for (i = 0; i < count; i++)
		{
			if (name && strcasecmp(name, e->names[i]))
			{
				return e->first + i;
			}
		}
	}
	while ((e = e->next));
	return -1;
}


