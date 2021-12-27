// CDI specific functions for list management

#ifndef CDI_LISTS
#define CDI_LISTS

#include "util/list.h"


/* Represents a list.
   The fields of this structure are implementation dependent. To access list elements, specific list functions must be used. */
typedef list_t* cdi_list_t;

/* Creates a new list.
   return: Returns a new list, or NULL if no memory could be allocated for the list. */
cdi_list_t cdi_list_create(void);

// Frees the memory associated with a list. (Values of the list members must already be free.)
void cdi_list_destroy(cdi_list_t list);

/* Adds a new element to the head of the list.
   list:   The list to insert into.
   value:  The element to be added.
   return: The list or NULL if value could not be inserted (for example, because no memory could be allocated). */
cdi_list_t cdi_list_push(cdi_list_t list, void* value);

/* Removes an element from the head of the list and returns that element's value.
   list:   The list from which the leading element (head) will be removed.
   return: The removed element, or NULL if the list was empty. */
void* cdi_list_pop(cdi_list_t list);

/* Tests if a list is empty.
   list:   The list to test.
   return: 1 if the list is empty, otherwise 0. */
size_t cdi_list_empty(cdi_list_t list);

/* Returns the value of a list element at a specified index.
   list:   The list from which index will be read
   index:  The index of the element to return the value of
   return: The list element requested or NULL if no element exists at the specified index. */
void* cdi_list_get(cdi_list_t list, size_t index);

/* Adds a new element (value) to the list. The element is added at index. The index of all elements before value shall remain the same,
   while the index of elements after value shall increase by one.
   list:   The list to insert into
   index:  The index to be used for the new element
   value:  The element to be inserted
   return: The newly ordered list, or NULL if the element could not be added (for example, because the index is too large). */
cdi_list_t cdi_list_insert(cdi_list_t list, size_t index, void* value);

/* Removes an element from a list.
   list:   The list to remove an element from
   index:  The index in the list that will be removed
   return: The element that was removed, or NULL if the element at the */
void* cdi_list_remove(cdi_list_t list, size_t index);

/* Get the size (length) of a list
   list: The list whose length is to be returned */
size_t cdi_list_size(cdi_list_t list);


#endif
