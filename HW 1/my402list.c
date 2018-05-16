#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>

#include "cs402.h"
#include "my402list.h"

int My402ListLength(My402List *aList) {
	int numElem = aList->num_members;
	return numElem;
}

int My402ListEmpty(My402List *aList) {
    if (aList->num_members != 0) {
		return FALSE;
	}
	else {
		return TRUE;
	}	
}

int My402ListAppend(My402List *aList, void *obj) {
    My402ListElem*element = (My402ListElem*)malloc(sizeof(My402ListElem));
	My402ListElem*lastElement = My402ListLast(aList);
	element->obj = obj;
    
	if (My402ListEmpty(aList)) {
		element->prev = &(aList->anchor);
		element->next = &(aList->anchor);
		(aList->anchor).prev = element;
		(aList->anchor).next = element;
		(aList->num_members++);
		return TRUE;
	}
	else {
		lastElement->next = element;
		element->prev = lastElement;
		element->next = &(aList->anchor);
		(aList->anchor).prev = element;	
		(aList->num_members)++;
        return TRUE;
    }
    return FALSE;
}

int My402ListPrepend(My402List *aList, void *obj) {
    My402ListElem*element = (My402ListElem*)malloc(sizeof(My402ListElem));
	My402ListElem*firstElement = My402ListFirst(aList);
	element->obj = obj;
	
	if (My402ListEmpty(aList)) { 
		element->prev = &(aList->anchor);
		element->next = &(aList->anchor);
		(aList->anchor).prev = element;
		(aList->anchor).next = element;
		(aList->num_members)++;
		return TRUE;
	}
	else {
		firstElement->prev = element;
		element->prev = &(aList->anchor);
		element->next = firstElement;
		(aList->anchor).next = element;	
		(aList->num_members)++;
		return TRUE;
	}
	return FALSE;
}

void My402ListUnlink(My402List *aList, My402ListElem *elem) {     
	My402ListElem *tempPrev = elem->prev;
	My402ListElem *tempNext = elem->next;
	tempPrev->next = tempNext; 
	tempNext->prev = tempPrev; 
	free(elem);
	(aList->num_members)--; 
}

void My402ListUnlinkAll(My402List*aList) {
	My402ListElem *element = NULL;
	for (element = My402ListFirst(aList); element != &(aList->anchor); element = My402ListNext(aList, element)) { 
		if (element == NULL) return;
		My402ListUnlink(aList, element);
	}
}

int My402ListInsertAfter(My402List *aList, void *obj, My402ListElem *elem) {
	My402ListElem*addedElem = (My402ListElem*)malloc(sizeof(My402ListElem));
	if (elem == NULL) {
		return My402ListAppend(aList, obj);
	} 
	else {
        addedElem->obj = obj;
        addedElem->prev = elem;
		(elem->next)->prev = addedElem;
		addedElem->next = (elem->next);
		elem->next = addedElem;
		(aList->num_members)++;
		return TRUE;
	}
	return FALSE;
}

int My402ListInsertBefore(My402List *aList, void *obj, My402ListElem *elem) {
	My402ListElem*addedElem = (My402ListElem*)malloc(sizeof(My402ListElem));
	if (elem == NULL) {
		return My402ListPrepend(aList, obj);
	} 
	else {
		addedElem->obj = obj;
		addedElem->prev = elem->prev;
		(elem->prev)->next = addedElem;
		elem->prev = addedElem;
		addedElem->next = elem;
		(aList->num_members)++;
		return TRUE;
	}
	return FALSE;
}

My402ListElem *My402ListFirst(My402List *aList) {
	if (aList->num_members != 0) {
        return (aList->anchor).next; 
	}                    
	else {
        return NULL;
	}
}

My402ListElem *My402ListLast(My402List *aList) {
	if (aList->num_members != 0) {
		return (aList->anchor).prev; 
	}                   
	else {
		return NULL;
	}
}

My402ListElem *My402ListNext(My402List *aList, My402ListElem *elem) {
	if (elem == My402ListLast(aList)) {	
		return NULL;
	}
	else {
		return (elem->next);
	}
}

My402ListElem *My402ListPrev(My402List *aList, My402ListElem *elem) {
	if (elem == My402ListFirst(aList)) {	
		return NULL;
	}
	else {
		return (elem->prev);   
	}
}

My402ListElem *My402ListFind(My402List *aList, void *obj) {
	My402ListElem *element = NULL;
	for (element = My402ListFirst(aList); element != &(aList->anchor); element = My402ListNext(aList, element)) {
		if(element == NULL) return NULL;
		if (element->obj == obj) {
			return element;
		}
	}	
	return NULL;
}

int My402ListInit(My402List *aList) {
	if (aList == NULL) {
		return FALSE;
	}
	My402ListUnlinkAll(aList);
	return TRUE;
}
