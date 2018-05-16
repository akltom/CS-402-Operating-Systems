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

#include "kernel.h"
#include "errno.h"
#include "globals.h"

#include "vm/vmmap.h"
#include "vm/shadow.h"
#include "vm/anon.h"

#include "proc/proc.h"

#include "util/debug.h"
#include "util/list.h"
#include "util/string.h"
#include "util/printf.h"

#include "fs/vnode.h"
#include "fs/file.h"
#include "fs/fcntl.h"
#include "fs/vfs_syscall.h"

#include "mm/slab.h"
#include "mm/page.h"
#include "mm/mm.h"
#include "mm/mman.h"
#include "mm/mmobj.h"
#include "mm/tlb.h"

static slab_allocator_t *vmmap_allocator;
static slab_allocator_t *vmarea_allocator;

void
vmmap_init(void)
{
        vmmap_allocator = slab_allocator_create("vmmap", sizeof(vmmap_t));
        KASSERT(NULL != vmmap_allocator && "failed to create vmmap allocator!");
        vmarea_allocator = slab_allocator_create("vmarea", sizeof(vmarea_t));
        KASSERT(NULL != vmarea_allocator && "failed to create vmarea allocator!");
}

vmarea_t *
vmarea_alloc(void)
{
        vmarea_t *newvma = (vmarea_t *) slab_obj_alloc(vmarea_allocator);
        if (newvma) {
                newvma->vma_vmmap = NULL;
        }
        return newvma;
}

void
vmarea_free(vmarea_t *vma)
{
        KASSERT(NULL != vma);
        slab_obj_free(vmarea_allocator, vma);
}

/* a debugging routine: dumps the mappings of the given address space. */
size_t
vmmap_mapping_info(const void *vmmap, char *buf, size_t osize)
{
        KASSERT(0 < osize);
        KASSERT(NULL != buf);
        KASSERT(NULL != vmmap);

        vmmap_t *map = (vmmap_t *)vmmap;
        vmarea_t *vma;
        ssize_t size = (ssize_t)osize;

        int len = snprintf(buf, size, "%21s %5s %7s %8s %10s %12s\n",
                           "VADDR RANGE", "PROT", "FLAGS", "MMOBJ", "OFFSET",
                           "VFN RANGE");

        list_iterate_begin(&map->vmm_list, vma, vmarea_t, vma_plink) {
                size -= len;
                buf += len;
                if (0 >= size) {
                        goto end;
                }

                len = snprintf(buf, size,
                               "%#.8x-%#.8x  %c%c%c  %7s 0x%p %#.5x %#.5x-%#.5x\n",
                               vma->vma_start << PAGE_SHIFT,
                               vma->vma_end << PAGE_SHIFT,
                               (vma->vma_prot & PROT_READ ? 'r' : '-'),
                               (vma->vma_prot & PROT_WRITE ? 'w' : '-'),
                               (vma->vma_prot & PROT_EXEC ? 'x' : '-'),
                               (vma->vma_flags & MAP_SHARED ? " SHARED" : "PRIVATE"),
                               vma->vma_obj, vma->vma_off, vma->vma_start, vma->vma_end);
        } list_iterate_end();

end:
        if (size <= 0) {
                size = osize;
                buf[osize - 1] = '\0';
        }
        /*
        KASSERT(0 <= size);
        if (0 == size) {
                size++;
                buf--;
                buf[0] = '\0';
        }
        */
        return osize - size;
}

/* Create a new vmmap, which has no vmareas and does
 * not refer to a process. */
vmmap_t *
vmmap_create(void)
{
        /*NOT_YET_IMPLEMENTED("VM: vmmap_create");*/
        dbg(DBG_PRINT, "(GRADING3B 1)\n");
        vmmap_t * new_vmmap = (vmmap_t *)slab_obj_alloc(vmmap_allocator);
        list_init(&(new_vmmap->vmm_list));
        new_vmmap->vmm_proc = NULL;
  
        return new_vmmap;
}

/* Removes all vmareas from the address space and frees the
 * vmmap struct. */
void
vmmap_destroy(vmmap_t *map)
{
        /*NOT_YET_IMPLEMENTED("VM: vmmap_destroy");*/
        KASSERT(NULL != map);
        dbg(DBG_PRINT, "(GRADING3A 3.a)\n");

        vmarea_t* vma = NULL;
  
        list_iterate_begin(&map->vmm_list, vma, vmarea_t, vma_plink) {
                dbg(DBG_PRINT, "(GRADING3B)\n");
                list_remove(&(vma->vma_plink));   
                list_remove(&(vma->vma_olink)); 
                vma->vma_obj->mmo_ops->put(vma->vma_obj);
                vma->vma_obj = NULL;
                vmarea_free(vma);
        } list_iterate_end();
  
        slab_obj_free(vmmap_allocator, map);
}

/* Add a vmarea to an address space. Assumes (i.e. asserts to some extent)
 * the vmarea is valid.  This involves finding where to put it in the list
 * of VM areas, and adding it. Don't forget to set the vma_vmmap for the
 * area. */
void
vmmap_insert(vmmap_t *map, vmarea_t *newvma)
{
        /*NOT_YET_IMPLEMENTED("VM: vmmap_insert");*/

        KASSERT(NULL != map && NULL != newvma);
        KASSERT(NULL == newvma->vma_vmmap);
        KASSERT(newvma->vma_start < newvma->vma_end);
        KASSERT(ADDR_TO_PN(USER_MEM_LOW) <= newvma->vma_start && ADDR_TO_PN(USER_MEM_HIGH) >= newvma->vma_end);
        dbg(DBG_PRINT, "(GRADING3A 3.b)\n");

        vmarea_t* vma, *temp = NULL;
        newvma->vma_vmmap = map;
        list_iterate_begin(&map->vmm_list, vma, vmarea_t, vma_plink) {
                if (vma->vma_start >= newvma->vma_start ) {
                        dbg(DBG_PRINT, "(GRADING3B)\n");
                        temp = vma;
                        list_insert_before(&temp->vma_plink, &newvma->vma_plink);
                        return;
                }
        } list_iterate_end();
        dbg(DBG_PRINT, "(GRADING3B)\n");
        list_insert_before(&map->vmm_list, &newvma->vma_plink);
}

/* Find a contiguous range of free virtual pages of length npages in
 * the given address space. Returns starting vfn for the range,
 * without altering the map. Returns -1 if no such range exists.
 *
 * Your algorithm should be first fit. If dir is VMMAP_DIR_HILO, you
 * should find a gap as high in the address space as possible; if dir
 * is VMMAP_DIR_LOHI, the gap should be as low as possible. */
int
vmmap_find_range(vmmap_t *map, uint32_t npages, int dir)
{
        /*NOT_YET_IMPLEMENTED("VM: vmmap_find_range");*/
        /*dbg(DBG_TEST, "called\n");*/
        vmarea_t* vma, *temp = NULL;
        int res = -1;
  
        if (dir == VMMAP_DIR_HILO) {
                dbg(DBG_PRINT, "(GRADING3B)\n");
                list_iterate_reverse(&map->vmm_list, vma, vmarea_t, vma_plink) {
                        if(temp) { 
                                                    
                                if (temp->vma_start - vma->vma_end >= npages) {
                                        dbg(DBG_PRINT, "(GRADING3C)\n");
                                        res = temp->vma_start - npages;
                                        break;
                                }
                                dbg(DBG_PRINT, "(GRADING3C)\n");
                        }
                        else {
                                if (ADDR_TO_PN(USER_MEM_HIGH) - vma->vma_end >= npages) {
                                        dbg(DBG_PRINT, "(GRADING3B)\n");
                                        res = ADDR_TO_PN(USER_MEM_HIGH) - npages;
                                        break;
                                }
                                dbg(DBG_PRINT, "(GRADING3C)\n");
                        }
                        temp = vma;
                } list_iterate_end();
  
                if (res == -1) {
                        /*
                        if (temp) {
                                if (temp->vma_start - ADDR_TO_PN(USER_MEM_LOW) >= npages) {
                                        res = temp->vma_start - npages;
                                        dbg(DBG_PRINT, "(GRADING3C 2)\n");
                                } else {
                                        dbg(DBG_PRINT, "(GRADING3C 2)\n");
                                }
                        }
                        else {
                                if (ADDR_TO_PN(USER_MEM_HIGH) - ADDR_TO_PN(USER_MEM_LOW) >= npages) {
                                        res = ADDR_TO_PN(USER_MEM_HIGH) - npages;
                                        dbg(DBG_TEST, "14\n");
                                } else {
                                        dbg(DBG_TEST, "15\n");
                                }
                        }
                        */
                        if (temp->vma_start - ADDR_TO_PN(USER_MEM_LOW) >= npages) {
                                res = temp->vma_start - npages;
                                dbg(DBG_PRINT, "(GRADING3C)\n");
                        } else {
                                dbg(DBG_PRINT, "(GRADING3C)\n");
                        }
                } else {
                        dbg(DBG_PRINT, "(GRADING3B)\n");
                }
        }
        /*
        else if (dir == VMMAP_DIR_LOHI) {
                list_iterate_begin(&map->vmm_list, vma, vmarea_t, vma_plink) {
                        if (temp) {

                                if (vma->vma_start - temp->vma_end >= npages) {
                                        res = temp->vma_end;
                                                            dbg(DBG_TEST, "17\n");
                                        break;
                                }
                                                    dbg(DBG_TEST, "18\n");
                        }
                        else {
                                if (vma->vma_start - ADDR_TO_PN(USER_MEM_LOW) >= npages) {
                                                            dbg(DBG_TEST, "19\n");
                                        res = ADDR_TO_PN(USER_MEM_LOW);
                                        break;
                                }
                                                    dbg(DBG_TEST, "20\n");
                        }
                        temp = vma;
                } list_iterate_end();
  
                if (res == -1) {
                                                                        dbg(DBG_TEST, "21\n");
                        if (temp) {
                                if (ADDR_TO_PN(USER_MEM_HIGH) - temp->vma_end >= npages) {
                                        dbg(DBG_TEST, "22\n");
                                        res = temp->vma_end;
                                }
                                else {
                                                    dbg(DBG_TEST, "23\n");
                                }
                        }
                        else {
                                if (ADDR_TO_PN(USER_MEM_HIGH) - ADDR_TO_PN(USER_MEM_LOW) >= npages) {
                                                                                            dbg(DBG_TEST, "24\n");
                                        res = ADDR_TO_PN(USER_MEM_LOW);
                                }
                                else {
                                                    dbg(DBG_TEST, "25\n");
                                }
                        }
                } else {
                                                    dbg(DBG_TEST, "26\n");
                }
        }
        */
        return res;
}

/* Find the vm_area that vfn lies in. Simply scan the address space
 * looking for a vma whose range covers vfn. If the page is unmapped,
 * return NULL. */
vmarea_t *
vmmap_lookup(vmmap_t *map, uint32_t vfn)
{
        /*NOT_YET_IMPLEMENTED("VM: vmmap_lookup");*/
        KASSERT(NULL != map);
        dbg(DBG_PRINT, "(GRADING3A 3.c)\n");
        vmarea_t* vma = NULL;
  
        list_iterate_begin(&map->vmm_list, vma, vmarea_t, vma_plink) {
  
                if (vfn >= vma->vma_start && vfn < vma->vma_end)
                {
                        dbg(DBG_PRINT, "(GRADING3B)\n");
                        return vma;
                }
  
        } list_iterate_end();

        dbg(DBG_PRINT, "(GRADING3C)\n");
  
        return NULL;
}

/* Allocates a new vmmap containing a new vmarea for each area in the
 * given map. The areas should have no mmobjs set yet. Returns pointer
 * to the new vmmap on success, NULL on failure. This function is
 * called when implementing fork(2). */
vmmap_t *
vmmap_clone(vmmap_t *map)
{
  
        vmmap_t* new_vmmap = vmmap_create();

  
        vmarea_t* vma = NULL;
  
        list_iterate_begin(&map->vmm_list, vma, vmarea_t, vma_plink) {
                dbg(DBG_PRINT, "(GRADING3B)\n");
                vmarea_t* vm = vmarea_alloc();
                vm->vma_start = vma->vma_start;
                vm->vma_end = vma->vma_end;
                vm->vma_off = vma->vma_off;
                vm->vma_prot = vma->vma_prot;
                vm->vma_flags = vma->vma_flags;
                vm->vma_vmmap = new_vmmap;
  
                list_link_init(&vm->vma_plink);
                list_link_init(&vm->vma_olink);                
  
                list_insert_tail(&new_vmmap->vmm_list, &vm->vma_plink);
  
        } list_iterate_end();
        dbg(DBG_PRINT, "(GRADING3B)\n");
        return new_vmmap;
}
/* Insert a mapping into the map starting at lopage for npages pages.
 * If lopage is zero, we will find a range of virtual addresses in the
 * process that is big enough, by using vmmap_find_range with the same
 * dir argument.  If lopage is non-zero and the specified region
 * contains another mapping that mapping should be unmapped.
 *
 * If file is NULL an anon mmobj will be used to create a mapping
 * of 0's.  If file is non-null that vnode's file will be mapped in
 * for the given range.  Use the vnode's mmap operation to get the
 * mmobj for the file; do not assume it is file->vn_obj. Make sure all
 * of the area's fields except for vma_obj have been set before
 * calling mmap.
 *
 * If MAP_PRIVATE is specified set up a shadow object for the mmobj.
 *
 * All of the input to this function should be valid (KASSERT!).
 * See mmap(2) for for description of legal input.
 * Note that off should be page aligned.
 *
 * Be very careful about the order operations are performed in here. Some
 * operation are impossible to undo and should be saved until there
 * is no chance of failure.
 *
 * If 'new' is non-NULL a pointer to the new vmarea_t should be stored in it.
 */
int
vmmap_map(vmmap_t *map, vnode_t *file, uint32_t lopage, uint32_t npages,
          int prot, int flags, off_t off, int dir, vmarea_t **new)
{
        /*NOT_YET_IMPLEMENTED("VM: vmmap_map");*/
        KASSERT(NULL != map);
        KASSERT(0 < npages);
        KASSERT((MAP_SHARED & flags) || (MAP_PRIVATE & flags));
        KASSERT((0 == lopage) || (ADDR_TO_PN(USER_MEM_LOW) <= lopage));
        KASSERT((0 == lopage) || (ADDR_TO_PN(USER_MEM_HIGH) >= (lopage + npages)));
        KASSERT(PAGE_ALIGNED(off));
        dbg(DBG_PRINT, "(GRADING3A 3.d)\n");

        vmarea_t *vma = vmarea_alloc();
        if (lopage == 0) {
                int ret  = vmmap_find_range(map, npages, dir);
                if (ret == -1)
                {
                        dbg(DBG_PRINT, "(GRADING3C)\n");
                        return -1;
                }
                dbg(DBG_PRINT, "(GRADING3B)\n");
                lopage = ret;
        }
        else {
                if(!vmmap_is_range_empty(map, lopage, npages)) {
                        dbg(DBG_PRINT, "(GRADING3B)\n");
                        vmmap_remove(map, lopage, npages);
                }
                else {
                    dbg(DBG_PRINT, "(GRADING3B)\n");
                }
        }
  
        vma->vma_start = lopage;
        vma->vma_end = lopage + npages;
        vma->vma_off = ADDR_TO_PN(off);
  
        vma->vma_prot = prot;
        vma->vma_flags = flags;
        vma->vma_obj = NULL;
        list_link_init(&vma->vma_plink);
        list_link_init(&vma->vma_olink);
  
        mmobj_t* vmmobj = NULL;
  
        if (file == NULL)
        {
                dbg(DBG_PRINT, "(GRADING3B)\n");
                vmmobj = anon_create();

        } else {
                int retval = file->vn_ops->mmap(file, vma, &vmmobj);
  
                dbg(DBG_PRINT, "(GRADING3B)\n");
        }
          
        if (flags & MAP_PRIVATE)
        {
                dbg(DBG_PRINT, "(GRADING3B)\n");
                mmobj_t * shadowMmobj=shadow_create();
  
                shadowMmobj->mmo_shadowed = vmmobj;
                      
                vma->vma_obj = shadowMmobj;
                
                mmobj_t *bottom_obj;
  
                bottom_obj = vmmobj;
  
                shadowMmobj->mmo_un.mmo_bottom_obj = bottom_obj;
           
                list_insert_head(&(bottom_obj->mmo_un.mmo_vmas), &(vma->vma_olink));
        }
          
        else {                        
                dbg(DBG_PRINT, "(GRADING3C)\n");
                vma->vma_obj = vmmobj;  
                list_insert_head(&(vmmobj->mmo_un.mmo_vmas), &(vma->vma_olink));
        }
        vmmap_insert(map, vma);
        if (new)
        {
                dbg(DBG_PRINT, "(GRADING3B)\n");
                *new = vma;
        } else {
                dbg(DBG_PRINT, "(GRADING3B)\n");
        }
        return 0;
}

/*
 * We have no guarantee that the region of the address space being
 * unmapped will play nicely with our list of vmareas.
 *
 * You must iterate over each vmarea that is partially or wholly covered
 * by the address range [addr ... addr+len). The vm-area will fall into one
 * of four cases, as illustrated below:
 *
 * key:
 *          [             ]   Existing VM Area
 *        *******             Region to be unmapped
 *
 * Case 1:  [   ******    ]
 * The region to be unmapped lies completely inside the vmarea. We need to
 * split the old vmarea into two vmareas. be sure to increment the
 * reference count to the file associated with the vmarea.
 *
 * Case 2:  [      *******]**
 * The region overlaps the end of the vmarea. Just shorten the length of
 * the mapping.
 *
 * Case 3: *[*****        ]
 * The region overlaps the beginning of the vmarea. Move the beginning of
 * the mapping (remember to update vma_off), and shorten its length.
 *
 * Case 4: *[*************]**
 * The region completely contains the vmarea. Remove the vmarea from the
 * list.
 */
int vmmap_remove_helper(vmarea_t* vma, uint32_t lopage, uint32_t npages) {
  
        if (lopage > vma->vma_start && lopage + npages < vma->vma_end )
        {
                dbg(DBG_PRINT, "(GRADING3C)\n");
                return 1;
        }
  
        if (lopage > vma->vma_start && lopage < vma->vma_end && lopage + npages >= vma->vma_end )
        {
                dbg(DBG_PRINT, "(GRADING3C)\n");
                return 2;
        }
  
        if (lopage <= vma->vma_start && lopage + npages < vma->vma_end && lopage + npages > vma->vma_start)
        {
                dbg(DBG_PRINT, "(GRADING3C)\n");
                return 3;
        }
  
        if (lopage <= vma->vma_start && lopage + npages >= vma->vma_end )
        {
                dbg(DBG_PRINT, "(GRADING3B)\n");
                return 4;
        }

        dbg(DBG_PRINT, "(GRADING3B)\n");
        return 0;
}
  
int
vmmap_remove(vmmap_t *map, uint32_t lopage, uint32_t npages)
{
  
        vmarea_t* vma = NULL;
  
        list_iterate_begin(&map->vmm_list, vma, vmarea_t, vma_plink) {
  
                int case_num = vmmap_remove_helper(vma, lopage, npages);
  
                if (case_num == 1) {
                        vmarea_t* newvma = vmarea_alloc(); 

                        newvma->vma_start = lopage + npages;
                        newvma->vma_end = vma->vma_end;
                        newvma->vma_off = vma->vma_off + lopage + npages - vma->vma_start;
                        newvma->vma_prot = vma->vma_prot;
                        newvma->vma_flags = vma->vma_flags;
                        newvma->vma_vmmap = map;
                        newvma->vma_obj = vma->vma_obj;
                        vma->vma_end = lopage;
                        /*vma->vma_obj->mmo_ops->ref(vma->vma_obj);*/
                          
                        vmarea_t *vma_after = list_item((vma->vma_plink).l_next, vmarea_t,
                                vma_plink);
                        list_insert_before(&(vma_after->vma_plink),
                                &(newvma->vma_plink));
                          
                        /*if (vma->vma_flags & MAP_PRIVATE) {*/
                        dbg(DBG_PRINT, "(GRADING3C)\n");
                        mmobj_t *o_obj = vma->vma_obj;
                        mmobj_t * shadow1 = shadow_create();

                        vma->vma_obj = shadow1;

                        mmobj_t * shadow2 = shadow_create();

                        newvma->vma_obj = shadow2;

                        newvma->vma_obj->mmo_un.mmo_bottom_obj = mmobj_bottom_obj(vma->vma_obj);
                        vma->vma_obj->mmo_un.mmo_bottom_obj =
                                newvma->vma_obj->mmo_un.mmo_bottom_obj;
                        vma->vma_obj->mmo_shadowed = o_obj;
                        newvma->vma_obj->mmo_shadowed = vma->vma_obj->mmo_shadowed;
                        o_obj->mmo_ops->ref(o_obj);
                        list_insert_tail(mmobj_bottom_vmas(o_obj), &(newvma->vma_olink));

                        /*}*/ 
                } else if (case_num == 2) {
                        vma->vma_end = lopage;
                        dbg(DBG_PRINT, "(GRADING3C)\n");
                } else if (case_num == 3) {
                        dbg(DBG_PRINT, "(GRADING3C)\n");
                        vma->vma_off += lopage + npages - vma->vma_start;
                        vma->vma_start = lopage + npages;
                } else if (case_num == 4) {
                        dbg(DBG_PRINT, "(GRADING3B)\n");
                        list_remove(&(vma->vma_plink)); 
                        list_remove(&(vma->vma_olink));   
                        vma->vma_obj->mmo_ops->put(vma->vma_obj);
                        vma->vma_obj = NULL;
                        vmarea_free(vma);
                } else {
                    dbg(DBG_PRINT, "(GRADING3B)\n");
                }
  
        } list_iterate_end();
        dbg(DBG_PRINT, "(GRADING3B)\n");
        tlb_flush_all();
        pt_unmap_range(curproc->p_pagedir, (uintptr_t)PN_TO_ADDR(lopage),
            (uintptr_t)PN_TO_ADDR(lopage + npages));
        return 0;
}

/*
 * Returns 1 if the given address space has no mappings for the
 * given range, 0 otherwise.
 */
int
vmmap_is_range_empty(vmmap_t *map, uint32_t startvfn, uint32_t npages)
{
        /*NOT_YET_IMPLEMENTED("VM: vmmap_is_range_empty");*/
  
        KASSERT((startvfn < startvfn + npages) && (ADDR_TO_PN(USER_MEM_LOW) <= startvfn) && (ADDR_TO_PN(USER_MEM_HIGH) >= startvfn + npages));
        dbg(DBG_PRINT, "(GRADING3A 3.d)\n");

        vmarea_t* vma = NULL;
        list_iterate_begin(&map->vmm_list, vma, vmarea_t, vma_plink) {
                if (!(vma->vma_start >= startvfn + npages || vma->vma_end <= startvfn))
                {
                        dbg(DBG_PRINT, "(GRADING3B)\n");
                        return 0;
                }
        } list_iterate_end();        
  
        dbg(DBG_PRINT, "(GRADING3B)\n");

        return 1;
}

/* Read into 'buf' from the virtual address space of 'map' starting at
 * 'vaddr' for size 'count'. To do so, you will want to find the vmareas
 * to read from, then find the pframes within those vmareas corresponding
 * to the virtual addresses you want to read, and then read from the
 * physical memory that pframe points to. You should not check permissions
 * of the areas. Assume (KASSERT) that all the areas you are accessing exist.
 * Returns 0 on success, -errno on error.
 */
int
vmmap_read(vmmap_t *map, const void *vaddr, void *buf, size_t count)
{
        int start_vfn = ADDR_TO_PN(vaddr);
        int start_off_set = PAGE_OFFSET(vaddr);
  
        int end_vfn = ADDR_TO_PN((int) vaddr + count);
        int end_off_set = PAGE_OFFSET((int) vaddr + count);
  
        if(start_vfn == end_vfn) {
                dbg(DBG_PRINT, "(GRADING3B)\n");
                vmarea_t *vma = vmmap_lookup(map, start_vfn);
                  
                KASSERT(vma);
  
                int vma_off = vma->vma_off;
  
                pframe_t* pf = NULL;
  
                int retval = pframe_lookup(vma->vma_obj, start_vfn-vma->vma_start + vma->vma_off ,1, &pf);
                
                memcpy(buf, ((char *)pf->pf_addr)+start_off_set, count);
                /*pframe_dirty(pf);*/
                dbg(DBG_PRINT, "(GRADING3B)\n");
                return 0;
        }
  
  
        int page_num = start_vfn;
  
        while (page_num <= end_vfn) {
                vmarea_t *vma = vmmap_lookup(map, page_num);
                  
                KASSERT(vma);
  
                int vma_off = vma->vma_off;
  
                pframe_t* pf = NULL;
  
                int retval = pframe_lookup(vma->vma_obj, page_num-vma->vma_start + vma->vma_off ,1, &pf);
  
                if (page_num == start_vfn)
                {
                        dbg(DBG_PRINT, "(GRADING3C)\n");
                        memcpy(buf, ((char *)pf->pf_addr)+start_off_set, PAGE_SIZE - start_off_set);
                        buf = ((char *) buf + PAGE_SIZE - start_off_set);
                }
                else {
                        dbg(DBG_PRINT, "(GRADING3C)\n");
                        memcpy(buf, pf->pf_addr, end_off_set);
                        buf = ((char *) buf + end_off_set);
                }
                /*
                else {
                                    dbg(DBG_TEST, "66\n");
                        memcpy(buf, pf->pf_addr, PAGE_SIZE);
                        buf = ((char *) buf + PAGE_SIZE);
  
                }*/
                ++page_num;
        }
        dbg(DBG_PRINT, "(GRADING3C)\n");
        return 0;
}

/* Write from 'buf' into the virtual address space of 'map' starting at
 * 'vaddr' for size 'count'. To do this, you will need to find the correct
 * vmareas to write into, then find the correct pframes within those vmareas,
 * and finally write into the physical addresses that those pframes correspond
 * to. You should not check permissions of the areas you use. Assume (KASSERT)
 * that all the areas you are accessing exist. Remember to dirty pages!
 * Returns 0 on success, -errno on error.
 */
int
vmmap_write(vmmap_t *map, void *vaddr, const void *buf, size_t count)
{
  
        int start_vfn = ADDR_TO_PN(vaddr);
        int start_off_set = PAGE_OFFSET(vaddr);
  
        int end_vfn = ADDR_TO_PN((int) vaddr + count - 1);
        int end_off_set = PAGE_OFFSET((int) vaddr + count - 1);
  
        if(start_vfn == end_vfn) {
                dbg(DBG_PRINT, "(GRADING3B)\n");
                vmarea_t *vma = vmmap_lookup(map, start_vfn);
                  
                KASSERT(vma);
  
                int vma_off = vma->vma_off;
  
                pframe_t* pf = NULL;
  
                int retval = pframe_lookup(vma->vma_obj, start_vfn-vma->vma_start + vma->vma_off ,1, &pf);
  
                memcpy(((char *)pf->pf_addr)+start_off_set, buf, count);
                pframe_dirty(pf);
                dbg(DBG_PRINT, "(GRADING3B)\n");
                return 0;
        }
        int page_num = start_vfn;
  
        while (page_num <= end_vfn) {
                vmarea_t *vma = vmmap_lookup(map, page_num);
                  
                KASSERT(vma);
  
                int vma_off = vma->vma_off;
  
                pframe_t* pf = NULL;
  
                int retval = pframe_lookup(vma->vma_obj, page_num-vma->vma_start + vma->vma_off ,1, &pf);
  
                if (page_num == start_vfn)
                {
                        dbg(DBG_PRINT, "(GRADING3B)\n");
                        memcpy(((char *)pf->pf_addr)+start_off_set, buf, PAGE_SIZE - start_off_set);
                        buf = ((char *) buf + PAGE_SIZE - start_off_set);
                }
                else
                {
                        dbg(DBG_PRINT, "(GRADING3B)\n");
                        memcpy(pf->pf_addr, buf, end_off_set);
                        buf = ((char *) buf + end_off_set);
                }
                /*else {
                                            dbg(DBG_TEST, "73\n");
                        memcpy(pf->pf_addr, buf, PAGE_SIZE);
                        buf = ((char *) buf + PAGE_SIZE);
                }
                */
                pframe_dirty(pf);
                ++page_num;
        }
  
        dbg(DBG_PRINT, "(GRADING3B)\n");
        return 0;
}
