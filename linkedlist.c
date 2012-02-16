#include "linkedlist.h"
#include <stdlib.h>
#include <stdio.h>
#include "bubbletrouble.h"

int generate_linkedlist_buf(linkedlist *l, char *data,
                            int (*func)(void *,char *)) {
	int len = 4;
	*(uint32_t *)data = htonl(l->len);
	linkedlist_node *cur = l->first;
	for (;cur;cur = cur->next) {
		len += func(cur->data,data+len);
	}
	return len;
}
int linkedlist_buf_size(linkedlist *l, int (*func)(void *)) {
	int len = 4;
	linkedlist_node *cur = l->first;
	for (;cur;cur = cur->next) {
		len += func(cur->data);
	}
	return len;
}
/**
 * Create the linked list
 */
linkedlist *linkedlist_create() {
	linkedlist *cur = malloc(sizeof(linkedlist));
	if (!cur) {
		ERR_ERRNO();
		return 0;
	}
	cur->len = 0;
	cur->first = 0;
	cur->last = 0;
	return cur;
}
/**
 * Create a duplicate list
 */
linkedlist *linkedlist_clone(linkedlist *l) {
	linkedlist *ll = malloc(sizeof(linkedlist));
	if (!ll) {
		ERR_ERRNO();
		return 0;
	}
	linkedlist_node *cur = l->first;
	for (;cur;cur = cur->next) {
		linkedlist_add_last(ll,cur->data);
	}
	return ll;
}
/**
 * Empty the list. Does not free data pointers
 */
void linkedlist_clear(linkedlist *l) {
	linkedlist_node *cur = l->first;
	for (;cur;) {
		linkedlist_node *tmp = cur;
		cur = cur->next;
		free(tmp);
	}
	l->len = 0;
	l->first = 0;
	l->last = 0;
}
/**
 * Empty the list then free the list
 */
void linkedlist_destroy(linkedlist *l) {
	linkedlist_node *cur = l->first;
	for (;cur;) {
		linkedlist_node *tmp = cur;
		cur = cur->next;
		free(tmp);
	}
	free(l);
}
/**
 * Empty the list, freeing the data, then free the list
 */
void linkedlist_destroy_full(linkedlist *l) {
	linkedlist_node *cur = l->first;
	for (;cur;) {
		linkedlist_node *tmp = cur;
		cur = cur->next;
		if (tmp->data)
			free(tmp->data);
		free(tmp);
	}
	free(l);
}
/**
 * Check if the following data pointer exists
 */
uint8_t linkedlist_contains(linkedlist *l, void *v) {
	linkedlist_node *next = l->first;
	for (;next;next = next->next) {
		if (next->data == v) {
			return 1;
		}
	}
	return 0;
}
/**
 * Add the item at the specified location.
 */
int linkedlist_add(linkedlist *l, void *v, int loc) {
	if (loc == l->len) {
		return linkedlist_add_last(l,v);
	} else if (loc == 0) {
		return linkedlist_add_first(l,v);
	} else if (loc > l->len || loc < 0) {
		return 0;
	}
	linkedlist_node *n = malloc(sizeof(linkedlist_node));
	if (!n) {
		ERR_ERRNO();
		return -1;
	}
	n->data = v;
	int a;
	linkedlist_node *next = l->first;
	for (a = 0;a < loc;a++)
		next = next->next;
	n->prev = next->prev;
	n->next = next;
	next->prev->next = n;
	next->prev = n;
	l->len++;
	return 0;
}
int linkedlist_add_first(linkedlist *l,void *v) {
	linkedlist_node *n = malloc(sizeof(linkedlist_node));
	if (!n) {
		ERR_ERRNO();
		return -1;
	}
	n->prev = 0;
	n->data = v;
	n->next = l->first;
	if (l->first)
		l->first->prev = n;
	else
		l->last = n;
	l->first = n;
	l->len++;
	return 0;
}
int linkedlist_add_last(linkedlist *l, void *v) {
	linkedlist_node *n = malloc(sizeof(linkedlist_node));
	if (!n) {
		ERR_ERRNO();
		return -1;
	}
	n->next = 0;
	n->data = v;
	n->prev = l->last;
	if (l->last)
		l->last->next = n;
	else
		l->first = n;
	l->last = n;
	l->len++;
	return 0;
}
void *linkedlist_get(linkedlist *l,int loc) {
	if (loc == l->len) {
		return linkedlist_get_last(l);
	} else if (loc == 0) {
		return linkedlist_get_first(l);
	} else if (loc > l->len || loc < 0) {
		return 0;
	}
	linkedlist_node *next = l->first;
	int a;
	for (a = 0;a < loc;a++)
		next = next->next;
	void *data = next->data;
	return data;
}
void *linkedlist_get_first(linkedlist *l) {
	if (!l->len) {
		return 0;
	}
	void *data = l->first->data;
	return data;
}
void *linkedlist_get_last(linkedlist *l) {
	if (!l->len) {
		return 0;
	}
	void *data = l->last->data;
	return data;
}
void linkedlist_remove(linkedlist *l,int loc) {
	if (loc == l->len) {
		linkedlist_remove_last(l);
		return;
	} else if (loc == 0) {
		linkedlist_remove_first(l);
		return;
	} else if (loc > l->len) {
		return;
	}
	linkedlist_node *next = l->first;
	int a;
	for (a = 0;a < loc;a++)
		next = next->next;
	next->prev->next = next->next;
	next->next->prev = next->prev;
	l->len--;
	free(next);
}
void linkedlist_remove_first(linkedlist *l) {
	if (l->len) {
		linkedlist_node *first = l->first;
		l->first = l->first->next;
		l->len--;
		if (l->len == 1)
			l->last = l->first;
		if (l->first)
			l->first->prev = 0;
		else
			l->last = 0;
		free(first);
	}
}
void linkedlist_remove_last(linkedlist *l) {
	if (l->len) {
		linkedlist_node *last = l->last;
		l->last = l->last->prev;
		l->len--;
		if (l->len == 1)
			l->first = l->last;
		if (l->last)
			l->last->next = 0;
		else
			l->first = 0;
		free(last);
	}
}
uint8_t linkedlist_remove_object(linkedlist *l,void *v) {
	int a;
	int loc = l->len;
	linkedlist_node *next = l->first;
	for (a = 0;a < loc;a++) {
		if (next->data == v) {
			break;
		}
		next = next->next;
	}
	if (!next) {
		return 0;
	}
	l->len--;
	if (next->next)
		next->next->prev = next->prev;
	else
		l->last = next->prev;
	if (next->prev)
		next->prev->next = next->next;
	else
		l->first = next->next;
	free(next);
	return 1;
}
void linkedlist_remove_node(linkedlist *l,linkedlist_node *n) {
	if (l->first == n)
		l->first = n->next;
	if (l->last == n)
		l->last = n->prev;
	if (n->prev)
		n->prev->next = n->next;
	if (n->next)
		n->next->prev = n->prev;
	l->len--;
}
int linkedlist_len(linkedlist *l) {
	return l->len;
}
/**
 * Appends the data elements one by one to the new list.
 */
int linkedlist_append_list(linkedlist *to, linkedlist *from) {//O(n)
	linkedlist_node *cur = from->first;
	for (;cur;cur = cur->next) {
		void *data = cur->data;
		if (linkedlist_add_last(to, data)) {
			cur = cur->prev;
			for (;cur;cur = cur->prev) {
				linkedlist_remove_last(to);
			}
			return -1;
		}
	}
	return 0;
}
/**
 * Destroys the origin list, but quickly concatenates the two lists.
 */
void linkedlist_concat_list(linkedlist *to,linkedlist *from) {//O(1)
	linkedlist_node *start = from->first;
	linkedlist_node *end = to->last;
	end->next = start;
	start->prev = end;
	to->last = from->last;
	free(from);
}

linkedlist_iterator *linkedlist_iterate(linkedlist *l) {
	linkedlist_iterator *i = malloc(sizeof(linkedlist_iterator));
	i->node = l->first;
	i->l = l;
	return i;
}
void linkedlist_iterator_next(linkedlist_iterator *i) {
	if (i->node && i->node->next)
		i->node = i->node->next;
}
void linkedlist_iterator_prev(linkedlist_iterator *i) {
	if (i->node && i->node->prev)
		i->node = i->node->prev;
}
char linkedlist_iterator_has_next(linkedlist_iterator *i) {
	if (i->node && i->node->next)
		return 1;
	return 0;
}
char linkedlist_iterator_has_prev(linkedlist_iterator *i) {
	if (i->node && i->node->prev)
		return 1;
	return 0;
}
void *linkedlist_iterator_get(linkedlist_iterator *i) {
	if (i->node)
		return i->node->data;
	return 0;
}
void linkedlist_iterator_remove(linkedlist_iterator *i) {
	if (!i->node)
		return;
	linkedlist_node *node = i->node;
	linkedlist *l = i->l;
	i->node = 0;
	if (!node->next) {
		l->last = node->prev;
		if (node->prev)
			node->prev->next = 0;
	} else {
		i->node = node->next;
		node->next->prev = node->prev;
		if (node->prev)
			node->prev->next = node->next;
	}
	if (!node->prev) {
		l->first = node->next;
		if (node->next)
			node->next->prev = 0;
	} else if (!i->node) {
		i->node = node->next;
		node->prev->next = node->next;
		if (node->next)
			node->next->prev = node->prev;
	}
}

char *linkedlist_buf_load(linkedlist *l, char *data,
                          char *(*func)(void *,char *,char *), char *end,
                          int size) {
	char *data_start = data;
	if (data + 4 < end) {
		fprintf(stderr,"Error in loading linkedlist. Dumping Stack.\nlinkedlist_buf_load(%p,%p,%p,%p,%i)\n",l,data,func,end,size);
		return 0;
	}
	int len = ntohl(*(uint32_t *)data);data += 4;
	int a;
	for (a = 0;a < len;a++) {
		void *something = malloc(size);
		data = func(something,data,end);
		linkedlist_add_last(l,something);
		if (!data) {
			fprintf(stderr,"linkedlist_buf_load(%p,%p,%p,%p,%i)\n",l,data,func,end,size);
			return 0;
		}
	}
	return data;
}
