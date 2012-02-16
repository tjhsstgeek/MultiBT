/**
\file linkedlist.h
*/

#ifndef __LINKEDLIST_H__
#define __LINKEDLIST_H__

#include <stdint.h>

/**
This is a node in a linkedlist. It holds pointers to the data and the nodes before and after it.
*/
typedef struct _linkedlist_node {
	/** This is a pointer to the data that needs to be saved in the list */
	void *data;
	/** This is a pointer to the previous node in the list. NULL if this is the first node. */
	struct _linkedlist_node *prev;
	/** This is a pointer to the next node in the list. NULL if this is the last node. */
	struct _linkedlist_node *next;
} linkedlist_node;

typedef struct {
	/** This is the length of the list. */
	int len;
	/** This is a pointer to the beginning node of the list. */
	linkedlist_node *first;
	/** This is a pointer to the ending node of the list. */
	linkedlist_node *last;
	//SDL_mutex *linkedlist_lock;
} linkedlist;

extern int generate_linkedlist_buf(linkedlist *,char *,int (*)(void *,char *));
extern int linkedlist_buf_size(linkedlist *,int (*)(void *));
/**
This creates a linkedlist. This also allocates anything the linkedlist requires.
\return A new empty linkedlist.
*/
extern linkedlist *linkedlist_create();
/**
This creates a new linkedlist and fills it with the same data as in the supplied linkedlist.
The clone linkedlist only depends on the data supplied from the other linkedlist.
\param linkedlist The linkedlist that will be cloned. 
\return The new cloned linklist.
*/
extern linkedlist *linkedlist_clone(linkedlist *);
/**
This empties the linkedlist. 
This will not destroy the data inside, so if this is the only reference to those objects, you will acquire memory leaks.
\param linkedlist The linkedlist list that should be emptied.
*/
extern void linkedlist_clear(linkedlist *);
/**
This function checks to see if the specified data exists inside the list.
\param linkedlist The linkedlist list that should be checked.
\param data The data that should be found inside the list
\return One if found, otherwise zero
*/
extern uint8_t linkedlist_contains(linkedlist *,void *);
/**
This destroys the linkedlist.
This frees up any memory used up by it.
This will not destroy the data held by it.
\param linkedlist The linkedlist that should be freed
*/
extern void linkedlist_destroy(linkedlist *);
/**
This destroys the linkedlist.
This frees up any memory used up by it.
This will destroy the data held by it.
\param linkedlist The linkedlist that should be freed
*/
extern void linkedlist_destroy_full(linkedlist *);
/**
 * This adds data at a specified location in a linkedlist.
 * If an invalid location is entered, nothing is inserted.
 * If the location is at the beginning or end, then add first or add last are used instead.
 * \param linkedlist The linkedlist to insert the data into
 * \param data The data to insert
 * \param location The index where to insert the data. 
 * \sa linkedlist_add_first,linkedlist_add_last
 * \return Nonzero on error.
*/
extern int linkedlist_add(linkedlist *, void *, int);
/**
This will insert the data at the beginning of the list. 
\param linkedlist The linkedlist to insert the data into
\param data The data to insert
*/
extern int linkedlist_add_first(linkedlist *,void *);
/**
This will insert the data at the end of the list. 
\param linkedlist The linkedlist to insert the data into
\param data The data to insert
*/
extern int linkedlist_add_last(linkedlist *,void *);
/**
This retrieves data from a specified location in the list.
Invalid locations return NULL.
If the location is at the beginning or end, then get first or get last are used instead.
\param linkedlist The linkedlist to retrieve data from
\param location The index from which to retrieve the data
\return The data that was retrieved from the list or NULL
\sa linkedlist_get_first,linkedlist_get_last
*/
extern void *linkedlist_get(linkedlist *,int);
/**
This retrieves data from the beginning of the list.
If the lit is empty, NULL is returned.
\param linkedlist The linkedlist to retrieve data from
\return The data that was retrieved from the list or NULL
*/
extern void *linkedlist_get_first(linkedlist *);
/**
This retrieves data from the end of the list.
If the lit is empty, NULL is returned.
\param linkedlist The linkedlist to retrieve data from
\return The data that was retrieved from the list or NULL
*/
extern void *linkedlist_get_last(linkedlist *);
extern void linkedlist_remove(linkedlist *,int);//Index specific
extern uint8_t linkedlist_remove_object(linkedlist *,void *);//Memory specific
extern void linkedlist_remove_first(linkedlist *);
extern void linkedlist_remove_last(linkedlist *);
extern int linkedlist_len(linkedlist *);
extern int linkedlist_append_list(linkedlist *,linkedlist *);//Take items from one linkedlist and copy them to the other
extern void linkedlist_concat_list(linkedlist *,linkedlist *);//Take items from one linkedlist and move them to the other

typedef struct {
	linkedlist *l;
	linkedlist_node *node;
} linkedlist_iterator;

extern linkedlist_iterator *linkedlist_iterate(linkedlist *);//Returns NULL if there is nothing to iterate
extern void linkedlist_iterator_next(linkedlist_iterator *);
extern void linkedlist_iterator_prev(linkedlist_iterator *);
extern char linkedlist_iterator_has_next(linkedlist_iterator *);
extern char linkedlist_iterator_has_prev(linkedlist_iterator *);
extern void *linkedlist_iterator_get(linkedlist_iterator *);
extern void linkedlist_iterator_remove(linkedlist_iterator *);

extern char *linkedlist_buf_load(linkedlist *,char *,char *(*)(void *,char *,char *),char *,int);


#endif
