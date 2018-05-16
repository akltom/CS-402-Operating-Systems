/******************************************************************************/
/* Important Spring 2017 CSCI 402 usage information:                          */
/*                                                                            */
/* This fils is part of CSCI 402 kernel programming assignments at USC.       */
/*         53616c7465645f5f2e8d450c0c5851acd538befe33744efca0f1c4f9fb5f       */
/*         3c8feabc561a99e53d4d21951738da923cd1c7bbd11b30a1afb11172f80b       */
/*         984b1acfbbf8fae6ea57e0583d2610a618379293cb1de8e1e9d07e6287e8       */
/*         de7e82f3d48866aa2009b599e92c852f7dbf7a6e573f1c7228ca34b9f368       */
/*         faaef0c0fcf294cb                                                   */
/* Please understand that you are NOT permitted to distribute or publically   */
/*         display a copy of this file (or ANY PART of it) for any reason.    */
/* If anyone (including your prospective employer) asks you to post the code, */
/*         you must inform them that you do NOT have permissions to do so.    */
/* You are also NOT permitted to remove or alter this comment block.          */
/* If this comment block is removed or altered in a submitted file, 20 points */
/*         will be deducted.                                                  */
/******************************************************************************/

#include "globals.h"
#include "errno.h"

#include "util/string.h"
#include "util/debug.h"

#include "mm/mmobj.h"
#include "mm/pframe.h"
#include "mm/mm.h"
#include "mm/page.h"
#include "mm/slab.h"
#include "mm/tlb.h"

#include "vm/vmmap.h"
#include "vm/shadow.h"
#include "vm/shadowd.h"

#define SHADOW_SINGLETON_THRESHOLD 5

int shadow_count = 0; /* for debugging/verification purposes */
#ifdef __SHADOWD__
/*
 * number of shadow objects with a single parent, that is another shadow
 * object in the shadow objects tree(singletons)
 */
static int shadow_singleton_count = 0;
#endif

static slab_allocator_t *shadow_allocator;

static void shadow_ref(mmobj_t *o);
static void shadow_put(mmobj_t *o);
static int  shadow_lookuppage(mmobj_t *o, uint32_t pagenum, int forwrite, pframe_t **pf);
static int  shadow_fillpage(mmobj_t *o, pframe_t *pf);
static int  shadow_dirtypage(mmobj_t *o, pframe_t *pf);
static int  shadow_cleanpage(mmobj_t *o, pframe_t *pf);

static mmobj_ops_t shadow_mmobj_ops = {
        .ref = shadow_ref,
        .put = shadow_put,
        .lookuppage = shadow_lookuppage,
        .fillpage  = shadow_fillpage,
        .dirtypage = shadow_dirtypage,
        .cleanpage = shadow_cleanpage
};

/*
 * This function is called at boot time to initialize the
 * shadow page sub system. Currently it only initializes the
 * shadow_allocator object.
 */
void
shadow_init()
{
        shadow_allocator = slab_allocator_create("shadow", sizeof(mmobj_t));
        KASSERT(NULL != shadow_allocator);
        dbg(DBG_PRINT, "(GRADING3A 6.a)\n");
}

/*
 * You'll want to use the shadow_allocator to allocate the mmobj to
 * return, then then initialize it. Take a look in mm/mmobj.h for
 * macros or functions which can be of use here. Make sure your initial
 * reference count is correct.
 */
mmobj_t *
shadow_create()
{
        dbg(DBG_PRINT, "(GRADING3B)\n");
        mmobj_t *shadowd_obj = (mmobj_t *)slab_obj_alloc(shadow_allocator);

        mmobj_init(shadowd_obj, &shadow_mmobj_ops);
        shadowd_obj->mmo_refcount = 1;
        return shadowd_obj;
}

/* Implementation of mmobj entry points: */

/*
 * Increment the reference count on the object.
 */
static void
shadow_ref(mmobj_t *o)
{
        KASSERT(o && (0 < o->mmo_refcount) && (&shadow_mmobj_ops == o->mmo_ops));
        dbg(DBG_PRINT,"(GRADING3A 6.b)\n");
        dbg(DBG_PRINT,"(GRADING3B)\n");
        o->mmo_refcount++;
}

/*
 * Decrement the reference count on the object. If, however, the
 * reference count on the object reaches the number of resident
 * pages of the object, we can conclude that the object is no
 * longer in use and, since it is a shadow object, it will never
 * be used again. You should unpin and uncache all of the object's
 * pages and then free the object itself.
 */
static void
shadow_put(mmobj_t *o)
{       
        KASSERT(o && (0 < o->mmo_refcount) && (&shadow_mmobj_ops == o->mmo_ops));
        dbg(DBG_PRINT, "(GRADING3A 6.c)\n");

        pframe_t *p;
        KASSERT(NULL != o && (&shadow_mmobj_ops == o->mmo_ops) && (0 < o->mmo_refcount));
        if(o->mmo_refcount-1 == o->mmo_nrespages){
                list_iterate_begin(&o->mmo_respages, p, pframe_t, pf_olink){
                        pframe_unpin(p);
                        pframe_clean(p);
                        dbg(DBG_PRINT, "(GRADING3B)\n");
                        pframe_free(p);
                }list_iterate_end();

                o->mmo_shadowed->mmo_ops->put(o->mmo_shadowed);

                slab_obj_free(shadow_allocator, o);
        }else {
                dbg(DBG_PRINT,"(GRADING3B)\n");
        }
        --o->mmo_refcount;
}


/* This function looks up the given page in this shadow object. The
 * forwrite argument is true if the page is being looked up for
 * writing, false if it is being looked up for reading. This function
 * must handle all do-not-copy-on-not-write magic (i.e. when forwrite
 * is false find the first shadow object in the chain which has the
 * given page resident). copy-on-write magic (necessary when forwrite
 * is true) is handled in shadow_fillpage, not here. It is important to
 * use iteration rather than recursion here as a recursive implementation
 * can overflow the kernel stack when looking down a long shadow chain */
static int
 shadow_lookuppage(mmobj_t *o, uint32_t pagenum, int forwrite, pframe_t **pf)
 {
        pframe_t *pg = NULL;
        mmobj_t *nextShadow_obj = o;

        if (forwrite == 1){
                dbg(DBG_PRINT,"(GRADING3B)\n");
                pg = pframe_get_resident(o, pagenum);

                if(pg != NULL){
                        dbg(DBG_PRINT,"(GRADING3B)\n");
                        *pf = pg;
                        return 0;
                }else{
                        dbg(DBG_PRINT, "(GRADING3B)\n");
                        int ret = pframe_get(o, pagenum, pf);
                        if(ret == 0){
                                dbg(DBG_PRINT,"(GRADING3B)\n");
                                shadow_dirtypage(o, *pf);
                                return 0;
                        }else{
                                dbg(DBG_PRINT,"(GRADING3D)\n");
                                return ret;
                        }
                }
        }else{
                dbg(DBG_PRINT,"(GRADING3B)\n");
                while (nextShadow_obj->mmo_shadowed != NULL){
                        pg = pframe_get_resident(nextShadow_obj, pagenum);

                        if(pg != NULL){
                                *pf = pg;
                                KASSERT(NULL != (*pf));
                                dbg(DBG_PRINT, "(GRADING3A 6.d)\n");
                                KASSERT((pagenum == (*pf)->pf_pagenum) && (!pframe_is_busy(*pf)));
                                dbg(DBG_PRINT, "(GRADING3A 6.d)\n");
                                return 0;
                        }else{
                                dbg(DBG_PRINT, "(GRADING3B)\n");
                                nextShadow_obj = nextShadow_obj->mmo_shadowed;
                        }
                }

                int result = pframe_lookup(nextShadow_obj, pagenum, 0, pf);

                if(result == 0){
                        KASSERT(NULL != (*pf));
                        dbg(DBG_PRINT, "(GRADING3A 6.d)\n");
                        KASSERT((pagenum == (*pf)->pf_pagenum) && (!pframe_is_busy(*pf)));
                        dbg(DBG_PRINT, "(GRADING3A 6.d)\n");
                        return 0;
                }else{
                        dbg(DBG_PRINT, "(GRADING3D)\n");
                        return result;
                }
        }
}

/* As per the specification in mmobj.h, fill the page frame starting
 * at address pf->pf_addr with the contents of the page identified by
 * pf->pf_obj and pf->pf_pagenum. This function handles all
 * copy-on-write magic (i.e. if there is a shadow object which has
 * data for the pf->pf_pagenum-th page then we should take that data,
 * if no such shadow object exists we need to follow the chain of
 * shadow objects all the way to the bottom object and take the data
 * for the pf->pf_pagenum-th page from the last object in the chain).
 * It is important to use iteration rather than recursion here as a 
 * recursive implementation can overflow the kernel stack when 
 * looking down a long shadow chain */
static int
shadow_fillpage(mmobj_t *o, pframe_t *pf)
{
        mmobj_t *nextShadow_obj = o;
        pframe_t *p = NULL;

        KASSERT(pframe_is_busy(pf));
        dbg(DBG_PRINT, "(GRADING3A 6.e)\n");
        KASSERT(!pframe_is_pinned(pf));
        dbg(DBG_PRINT, "(GRADING3A 6.e)\n");


        while(nextShadow_obj->mmo_shadowed != NULL){
                dbg(DBG_PRINT, "(GRADING3B)\n");
                
                p = pframe_get_resident(nextShadow_obj->mmo_shadowed, pf->pf_pagenum);
                if(p != NULL){
                                          
                        dbg(DBG_PRINT,"(GRADING3B)\n");
                        pframe_pin(pf);
                        memcpy(pf->pf_addr, p->pf_addr, PAGE_SIZE);
                        return 0;

                }else{
                        dbg(DBG_PRINT, "(GRADING3B)\n");
                        nextShadow_obj = nextShadow_obj->mmo_shadowed;
                }
        }

        int ret = pframe_lookup(o->mmo_un.mmo_bottom_obj, pf->pf_pagenum,0, &p);

        if(ret == 0){
                dbg(DBG_PRINT,"(GRADING3B)\n");
                pframe_pin(pf);
                memcpy(pf->pf_addr, p->pf_addr, PAGE_SIZE);
                return 0;
        }else{
                dbg(DBG_PRINT,"(GRADING3D)\n");
                return ret;
        }
}

/* These next two functions are not difficult. */

static int
shadow_dirtypage(mmobj_t *o, pframe_t *pf)
{
        dbg(DBG_PRINT,"(GRADING3B)\n");
        pframe_set_dirty(pf);
        return 0;
}

static int
shadow_cleanpage(mmobj_t *o, pframe_t *pf)
{
        dbg(DBG_PRINT, "(GRADING3B)\n");
        pframe_t  *pg;
        o->mmo_ops->lookuppage(o, pf->pf_pagenum, 1, &pg);
        memcpy(pg->pf_addr,pf->pf_addr,PAGE_SIZE);
        pframe_clear_dirty(pg);
        return 0;
}
