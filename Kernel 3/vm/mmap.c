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
#include "types.h"

#include "mm/mm.h"
#include "mm/tlb.h"
#include "mm/mman.h"
#include "mm/page.h"

#include "proc/proc.h"

#include "util/string.h"
#include "util/debug.h"

#include "fs/vnode.h"
#include "fs/vfs.h"
#include "fs/file.h"

#include "vm/vmmap.h"
#include "vm/mmap.h"

/*
 * This function implements the mmap(2) syscall, but only
 * supports the MAP_SHARED, MAP_PRIVATE, MAP_FIXED, and
 * MAP_ANON flags.
 *
 * Add a mapping to the current process's address space.
 * You need to do some error checking; see the ERRORS section
 * of the manpage for the problems you should anticipate.
 * After error checking most of the work of this function is
 * done by vmmap_map(), but remember to clear the TLB.
 */
int
do_mmap(void *addr, size_t len, int prot, int flags,
        int fd, off_t off, void **ret)
{
        /*NOT_YET_IMPLEMENTED("VM: do_mmap");*/

        uint32_t lopage;
 
        if ((flags & MAP_FIXED))
        {
                dbg(DBG_PRINT, "(GRADING3B)\n");
                if (USER_MEM_LOW > (unsigned int) addr || (USER_MEM_HIGH < (unsigned int) (addr) + len))
                {
                        dbg(DBG_PRINT, "(GRADING3D)\n");
                        return -EINVAL;
                }
                lopage = ADDR_TO_PN(addr);
        } else {
                dbg(DBG_PRINT, "(GRADING3B)\n");
                lopage = 0;
        }
 
        if (!PAGE_ALIGNED(off) || len <= 0 || len > (USER_MEM_HIGH - USER_MEM_LOW))
        {
                dbg(DBG_PRINT, "(GRADING3D)\n");
                return -EINVAL; 
        }
 
        if (!((flags & MAP_SHARED) ^ (flags & MAP_PRIVATE)))
        {
                dbg(DBG_PRINT, "(GRADING3D)\n");
                return -EINVAL;
        }
 
        file_t *ft = NULL;
        vmarea_t *vm = NULL;
        int retval;

        if ((flags & MAP_ANON) != MAP_ANON)
        {
                if (fd > NFILES || fd < 0)
                {
                        dbg(DBG_PRINT, "(GRADING3D)\n");
                        return -EBADF;
                }
 
                ft = fget(fd); 
                if (ft == NULL)
                {
                        dbg(DBG_PRINT, "(GRADING3D)\n");
                        return -EBADF;
                }

                if ((flags & MAP_SHARED) && (prot & PROT_WRITE) && (ft->f_mode == FMODE_READ))
                {       
                        dbg(DBG_PRINT, "(GRADING3D)\n");
                        fput(ft);
                        return -EINVAL;
                }

                retval = vmmap_map(curproc->p_vmmap, ft->f_vnode, lopage, (len - 1) / PAGE_SIZE + 1, 
                prot, flags, off, VMMAP_DIR_HILO, &vm);
                        
                dbg(DBG_PRINT, "(GRADING3B)\n");
                fput(ft);
                
        }
        else {
                dbg(DBG_PRINT, "(GRADING3B)\n");
                retval = vmmap_map(curproc->p_vmmap, 0, lopage,  (len - 1) / PAGE_SIZE + 1, 
                prot, flags, off, VMMAP_DIR_HILO, &vm);
        }
 
        if (retval < 0)
        {
                dbg(DBG_PRINT, "(GRADING3D)\n");
                return retval;
        }

        *ret = PN_TO_ADDR(vm->vma_start);
     
        pt_unmap_range(curproc->p_pagedir, (uintptr_t) PN_TO_ADDR(vm->vma_start),
               (uintptr_t) PN_TO_ADDR(vm->vma_start)
               + (uintptr_t) PAGE_ALIGN_UP(len));
    
        tlb_flush_range((uintptr_t) PN_TO_ADDR(vm->vma_start),
                (uint32_t) PAGE_ALIGN_UP(len) / PAGE_SIZE);

        KASSERT(NULL != curproc->p_pagedir);
        dbg(DBG_PRINT, "(GRADING3A 2.a)\n");
 
        return retval;
}


/*
 * This function implements the munmap(2) syscall.
 *
 * As with do_mmap() it should perform the required error checking,
 * before calling upon vmmap_remove() to do most of the work.
 * Remember to clear the TLB.
 */
int
do_munmap(void *addr, size_t len)
{
        
        unsigned int lopage = ADDR_TO_PN(addr);
        if (USER_MEM_LOW > (unsigned int) addr)
        {
                dbg(DBG_PRINT, "(GRADING3D)\n");
                return -EINVAL;
        }
 
        if (USER_MEM_HIGH < ((unsigned int)addr + len))
        {
                dbg(DBG_PRINT, "(GRADING3D)\n");
                return -EINVAL;
        }
 /*
        if (!PAGE_ALIGNED(addr))
        {
                    dbg(DBG_TEST, "18\n");
                return -EINVAL; 
        }
 */
        if (len <= 0 || len > (USER_MEM_HIGH - USER_MEM_LOW))
        {
                dbg(DBG_PRINT, "(GRADING3D)\n");
                return -EINVAL; 
        }
 
        int retval = vmmap_remove(curproc->p_vmmap, lopage, 
            (len - 1) / PAGE_SIZE + 1);
 /*
        if (retval < 0)
        {
                    dbg(DBG_TEST, "20\n");
                return retval;
        }
 */
        dbg(DBG_PRINT, "(GRADING3B)\n");
        tlb_flush_all();
 
        return 0;
}

