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

int anon_count = 0; /* for debugging/verification purposes */

static slab_allocator_t *anon_allocator;

static void anon_ref(mmobj_t *o);
static void anon_put(mmobj_t *o);
static int  anon_lookuppage(mmobj_t *o, uint32_t pagenum, int forwrite, pframe_t **pf);
static int  anon_fillpage(mmobj_t *o, pframe_t *pf);
static int  anon_dirtypage(mmobj_t *o, pframe_t *pf);
static int  anon_cleanpage(mmobj_t *o, pframe_t *pf);

static mmobj_ops_t anon_mmobj_ops = {
        .ref = anon_ref,
        .put = anon_put,
        .lookuppage = anon_lookuppage,
        .fillpage  = anon_fillpage,
        .dirtypage = anon_dirtypage,
        .cleanpage = anon_cleanpage
};

/*
 * This function is called at boot time to initialize the
 * anonymous page sub system. Currently it only initializes the
 * anon_allocator object.
 */
void
anon_init()
{
        /*NOT_YET_IMPLEMENTED("VM: anon_init");*/
        
        anon_allocator = slab_allocator_create("anon",sizeof(mmobj_t));
        
        KASSERT(anon_allocator);

        dbg(DBG_PRINT, "(GRADING3A 4.a)\n");
              
}

/*
 * You'll want to use the anon_allocator to allocate the mmobj to
 * return, then then initialize it. Take a look in mm/mmobj.h for
 * macros which can be of use here. Make sure your initial
 * reference count is correct.
 */
mmobj_t *
anon_create()
{

        mmobj_t *mmobj = (mmobj_t*) slab_obj_alloc(anon_allocator);
        mmobj_init(mmobj, &anon_mmobj_ops);
        mmobj->mmo_refcount = 1;
        dbg(DBG_PRINT, "(GRADING3B)\n");
        return mmobj;

}

/* Implementation of mmobj entry points: */

/*
 * Increment the reference count on the object.
 */
static void
anon_ref(mmobj_t *o)
{      
        KASSERT(o && (0 < o->mmo_refcount) && (&anon_mmobj_ops == o->mmo_ops));
        dbg(DBG_PRINT, "(GRADING3A 4.b)\n");
        o->mmo_refcount++;
}

/*
 * Decrement the reference count on the object. If, however, the
 * reference count on the object reaches the number of resident
 * pages of the object, we can conclude that the object is no
 * longer in use and, since it is an anonymous object, it will
 * never be used again. You should unpin and uncache all of the
 * object's pages and then free the object itself.
 */
static void
anon_put(mmobj_t *o)
{
       
        KASSERT(o && (0 < o->mmo_refcount) && (&anon_mmobj_ops == o->mmo_ops));

        dbg(DBG_PRINT, "(GRADING3A 4.c)\n");
       

        if(o->mmo_refcount-1 == o->mmo_nrespages) {  
                pframe_t* pf;
                list_iterate_begin(&o->mmo_respages, pf, pframe_t, pf_olink) {
                        
                        if(pframe_is_dirty (pf) ) {
                                dbg(DBG_PRINT, "(GRADING3D)\n");
                                pframe_clean(pf);
                        }

                        dbg(DBG_PRINT, "(GRADING3D)\n");
                        pframe_free(pf); 

                } list_iterate_end();

                slab_obj_free(anon_allocator, o); 
                
        }
        dbg(DBG_PRINT, "(GRADING3B)\n");
        o->mmo_refcount--; 

}

/* Get the corresponding page from the mmobj. No special handling is
 * required. */
static int
anon_lookuppage(mmobj_t *o, uint32_t pagenum, int forwrite, pframe_t **pf)
{
        int page = pframe_get(o, pagenum, pf);
        dbg(DBG_PRINT, "(GRADING3B)\n");
        return page;
}

/* The following three functions should not be difficult. */

static int
anon_fillpage(mmobj_t *o, pframe_t *pf)
{
        
        KASSERT(pframe_is_busy(pf));
        dbg(DBG_PRINT, "(GRADING3A 4.d)\n");
        KASSERT(!pframe_is_pinned(pf));
        dbg(DBG_PRINT, "(GRADING3A 4.d)\n");

        pframe_pin(pf);
        
        memset(pf->pf_addr, 0, PAGE_SIZE);
       
        pframe_unpin(pf);
        
        dbg(DBG_PRINT, "(GRADING3B)\n");
        return 0;
}

static int
anon_dirtypage(mmobj_t *o, pframe_t *pf)
{
    
        dbg(DBG_PRINT, "(GRADING3D)\n");
        pframe_set_dirty(pf);

        return 1;
}

static int
anon_cleanpage(mmobj_t *o, pframe_t *pf)
{
        
        dbg(DBG_PRINT, "(GRADING3D)\n");
        pframe_clear_dirty(pf);

        return 1;
}
