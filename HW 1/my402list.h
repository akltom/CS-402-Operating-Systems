/******************************************************************************/
/* Important CSCI 402 usage information:                                      */
/*                                                                            */
/* This fils is part of CSCI 402 warmup programming assignments at USC.       */
/* Please understand that you are NOT permitted to distribute or publically   */
/*         display a copy of this file (or ANY PART of it) for any reason.    */
/* If anyone (including your prospective employer) asks you to post the code, */
/*         you must inform them that you do NOT have permissions to do so.    */
/* You are also NOT permitted to remove or alter this comment block.          */
/******************************************************************************/

/*
 * Author:      William Chia-Wei Cheng (bill.cheng@acm.org)
 *
 * @(#)$Id: my402list.h,v 1.2 2012/09/04 12:51:51 william Exp $
 */

#ifndef _MY402LIST_H_
#define _MY402LIST_H_

#include "cs402.h"

typedef struct tagMy402ListElem {
    void *obj;
    struct tagMy402ListElem *next;
    struct tagMy402ListElem *prev;
} My402ListElem;

typedef struct tagMy402List {
    int num_members;
    My402ListElem anchor;

    /* You do not have to set these function pointers */
    int  (*Length)(struct tagMy402List *);
    int  (*Empty)(struct tagMy402List *);

    int  (*Append)(struct tagMy402List *, void*);
    int  (*Prepend)(struct tagMy402List *, void*);
    void (*Unlink)(struct tagMy402List *, My402ListElem*);
    void (*UnlinkAll)(struct tagMy402List *);
    int  (*InsertBefore)(struct tagMy402List *, void*, My402ListElem*);
    int  (*InsertAfter)(struct tagMy402List *, void*, My402ListElem*);

    My402ListElem *(*First)(struct tagMy402List *);
    My402ListElem *(*Last)(struct tagMy402List *);
    My402ListElem *(*Next)(struct tagMy402List *, My402ListElem *cur);
    My402ListElem *(*Prev)(struct tagMy402List *, My402ListElem *cur);

    My402ListElem *(*Find)(struct tagMy402List *, void *obj);
} My402List;

extern int  My402ListLength(My402List*);
extern int  My402ListEmpty(My402List*);

extern int  My402ListAppend(My402List*, void*);
extern int  My402ListPrepend(My402List*, void*);
extern void My402ListUnlink(My402List*, My402ListElem*);
extern void My402ListUnlinkAll(My402List*);
extern int  My402ListInsertAfter(My402List*, void*, My402ListElem*);
extern int  My402ListInsertBefore(My402List*, void*, My402ListElem*);

extern My402ListElem *My402ListFirst(My402List*);
extern My402ListElem *My402ListLast(My402List*);
extern My402ListElem *My402ListNext(My402List*, My402ListElem*);
extern My402ListElem *My402ListPrev(My402List*, My402ListElem*);

extern My402ListElem *My402ListFind(My402List*, void*);

extern int My402ListInit(My402List*);

#endif /*_MY402LIST_H_*/
