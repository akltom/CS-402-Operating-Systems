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

#include "types.h"
#include "globals.h"
#include "kernel.h"
#include "errno.h"

#include "util/debug.h"

#include "proc/proc.h"

#include "mm/mm.h"
#include "mm/mman.h"
#include "mm/page.h"
#include "mm/mmobj.h"
#include "mm/pframe.h"
#include "mm/pagetable.h"

#include "vm/pagefault.h"
#include "vm/vmmap.h"
#include "mm/tlb.h"

/*
 * This gets called by _pt_fault_handler in mm/pagetable.c The
 * calling function has already done a lot of error checking for
 * us. In particular it has checked that we are not page faulting
 * while in kernel mode. Make sure you understand why an
 * unexpected page fault in kernel mode is bad in Weenix. You
 * should probably read the _pt_fault_handler function to get a
 * sense of what it is doing.
 *
 * Before you can do anything you need to find the vmarea that
 * contains the address that was faulted on. Make sure to check
 * the permissions on the area to see if the process has
 * permission to do [cause]. If either of these checks does not
 * pass kill the offending process, setting its exit status to
 * EFAULT (normally we would send the SIGSEGV signal, however
 * Weenix does not support signals).
 *
 * Now it is time to find the correct page (don't forget
 * about shadow objects, especially copy-on-write magic!). Make
 * sure that if the user writes to the page it will be handled
 * correctly.
 *
 * Finally call pt_map to have the new mapping placed into the
 * appropriate page table.
 *
 * @param vaddr the address that was accessed to cause the fault
 *
 * @param cause this is the type of operation on the memory
 *              address which caused the fault, possible values
 *              can be found in pagefault.h
 */
void
handle_pagefault(uintptr_t vaddr, uint32_t cause)
{
    uint32_t pn = ADDR_TO_PN(vaddr);
    uint32_t ptFlag = PT_PRESENT | PT_USER;
    uint32_t pdFlag = PD_PRESENT | PD_USER;

    vmarea_t *fault_vmarea = vmmap_lookup(curproc->p_vmmap, pn);

    /*check if vmarea of curproc does not exist*/
    if(fault_vmarea == NULL){
            dbg(DBG_PRINT,"(GRADING3D)\n");
            do_exit(EFAULT);
    }

    if((cause & FAULT_WRITE) && !(fault_vmarea->vma_prot & PROT_WRITE)){
            dbg(DBG_PRINT,"(GRADING3D)\n");
            do_exit(EFAULT);
    }
    /*not included in self check*/
    /*
    if((cause & FAULT_EXEC) && !(fault_vmarea->vma_prot & PROT_EXEC)){
        dbg(DBG_ERROR, "ccccccccccc\n");
        do_exit(EFAULT);
    }
    */
    /*not included in self check*/
    /*
    if (fault_vmarea->vma_prot & PROT_NONE){
        dbg(DBG_ERROR, "dddddddddddddddddddddd\n");
        do_exit(EFAULT);
    }
    */
    if (!((cause & FAULT_WRITE) || (cause & FAULT_EXEC)) && !(fault_vmarea->vma_prot & PROT_READ)){
            dbg(DBG_PRINT, "(GRADING3D)\n");
            do_exit(EFAULT);
    }

    pframe_t *result_pf;
    int forwrite = 0;

    if(cause & FAULT_WRITE){
            dbg(DBG_PRINT,"(GRADING3B)\n");
            forwrite = 1;
            ptFlag = ptFlag | PT_WRITE;
            pdFlag = pdFlag | PD_WRITE;
    }

    if(pframe_lookup(fault_vmarea->vma_obj, pn - fault_vmarea->vma_start + fault_vmarea->vma_off, forwrite, &result_pf) != 0){
            dbg(DBG_PRINT,"(GRADING3D)\n");
            do_exit(EFAULT);
    }else{
            KASSERT(result_pf);
            dbg(DBG_PRINT,"(GRADING3A 5.a)\n");
            KASSERT(result_pf->pf_addr);
            dbg(DBG_PRINT,"(GRADING3A 5.a)\n");

            if(forwrite == 1){
                    dbg(DBG_PRINT,"(GRADING3B)\n");
                    pframe_pin(result_pf);
                    pframe_dirty(result_pf);
                    pframe_unpin(result_pf);
            }

            dbg(DBG_PRINT,"(GRADING3B)\n");

            uintptr_t paddr = (uintptr_t)pt_virt_to_phys((uintptr_t)result_pf->pf_addr);
            int map_result = pt_map(curproc->p_pagedir, (uintptr_t)PAGE_ALIGN_DOWN(vaddr), paddr, pdFlag, ptFlag);

            tlb_flush((uintptr_t)PAGE_ALIGN_DOWN(vaddr));
    }
}
