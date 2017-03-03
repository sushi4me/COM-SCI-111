#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <sched.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "SortedList.h"

int opt_yield = 0;

void SortedList_insert(SortedList_t *list, SortedListElement_t *element) {
	// Return if list or element is pointing to NULL
	if(!list || !element)
		return;
	SortedListElement_t *temp = list->next;
	SortedListElement_t *curr = list;
	// List is empty; there is only a HEAD
    	if(!temp) {
        	if(opt_yield & INSERT_YIELD)
            		sched_yield();
        	list->next = element;
		element->next = NULL;
        	element->prev = list;
        	return;
	}
	// While the element key is LESS THAN the next key, place element 
    	while(temp && strcmp(element->key, temp->key) >= 0) {
		curr = temp;        
		temp = temp->next;
	}
	// Check before placing new element
    	if(opt_yield & INSERT_YIELD)
        	sched_yield();
    	// We reached the end of the list
    	if(!temp) {
        	curr->next = element;
        	element->prev = curr;
        	element->next = NULL;
    	} // We found the spot element belongs
    	else {
		curr->next = element;
		temp->prev = element;
		element->next = temp;
		element->prev = curr;
	}
}

int SortedList_delete(SortedListElement_t *element) {
	bool flag = false;
	// Element is NULL	
	if(!element)
		return 1;
	// Check for critical section
	if(opt_yield & DELETE_YIELD)
		sched_yield();
    	// Next pointer is not NULL
    	if(element->next) {
		// Corrupted pointer
        	if(element->next->prev != element)
            		return 1;
		else {
			flag = true;
			element->next->prev = element->prev;
		}	
	}
    	// Prev pointer is not NULL
    	if(element->prev) {
        	if(element->prev->next != element) {
			// Revert the change from prior if done
			if(flag)
				element->next->prev = element;
            		return 1;
		}
        	else
            		element->prev->next = element->next;
    	}
	return 0;
}

SortedListElement_t *SortedList_lookup(SortedList_t *list, const char *key) {
	// If the list is NULL
	if (!list)
		return NULL;

	SortedListElement_t *temp = list->next;

	// Loop while it a is not null
	while(temp) {
		// Check for yield
		if(opt_yield & LOOKUP_YIELD)
			sched_yield();
		// Key found
		if(strcmp(key, temp->key) == 0)
			return temp;
		temp = temp->next;
	}
	return NULL;
}

int SortedList_length(SortedList_t *list) {
	// Check for NULL parameter
	if(!list)
		return -1;
	// Start with item right after HEAD
	SortedListElement_t *temp = list->next;	
	int length = 0;
	// Start search
	while(temp) {
		if (opt_yield & LOOKUP_YIELD)
			sched_yield();
		if (temp->prev != NULL && temp->prev->next != temp)
			return -1;
		if (temp->next != NULL && temp->next->prev != temp)
			return -1;
		length++;
		temp = temp->next;
	}
	return length;
}
