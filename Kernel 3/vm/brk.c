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
#include "util/debug.h"

#include "mm/mm.h"
#include "mm/page.h"
#include "mm/mman.h"

#include "vm/mmap.h"
#include "vm/vmmap.h"

#include "proc/proc.h"

/*
 * This function implements the brk(2) system call.
 *
 * This routine manages the calling process's "break" -- the ending address
 * of the process's "dynamic" region (often also referred to as the "heap").
 * The current value of a process's break is maintained in the 'p_brk' member
 * of the proc_t structure that represents the process in question.
 *
 * The 'p_brk' and 'p_start_brk' members of a proc_t struct are initialized
 * by the loader. 'p_start_brk' is subsequently never modified; it always
 * holds the initial value of the break. Note that the starting break is
 * not necessarily page aligned!
 *
 * 'p_start_brk' is the lower limit of 'p_brk' (that is, setting the break
 * to any value less than 'p_start_brk' should be disallowed).
 *
 * The upper limit of 'p_brk' is defined by the minimum of (1) the
 * starting address of the next occuring mapping or (2) USER_MEM_HIGH.
 * That is, growth of the process break is limited only in that it cannot
 * overlap with/expand into an existing mapping or beyond the region of
 * the address space allocated for use by userland. (note the presence of
 * the 'vmmap_is_range_empty' function).
 *
 * The dynamic region should always be represented by at most ONE vmarea.
 * Note that vmareas only have page granularity, you will need to take this
 * into account when deciding how to set the mappings if p_brk or p_start_brk
 * is not page aligned.
 *
 * You are guaranteed that the process data/bss region is non-empty.
 * That is, if the starting brk is not page-aligned, its page has
 * read/write permissions.
 *
 * If addr is NULL, you should NOT fail as the man page says. Instead,
 * "return" the current break. We use this to implement sbrk(0) without writing
 * a separate syscall. Look in user/libc/syscall.c if you're curious.
 *
 * Also, despite the statement on the manpage, you MUST support combined use
 * of brk and mmap in the same process.
 *
 * Note that this function "returns" the new break through the "ret" argument.
 * Return 0 on success, -errno on failure.
 */
int
do_brk(void *addr, void **ret)
{

        if(addr == NULL) {
                dbg(DBG_PRINT, "(GRADING3B)\n");
            	*ret = curproc->p_brk;
            	return 0;
        }

        if(addr < curproc->p_start_brk || (unsigned int)addr > USER_MEM_HIGH) {
            	dbg(DBG_PRINT,"(GRADING3E)\n");
            	return -ENOMEM;
        }
        
        vmmap_t *map = curproc->p_vmmap;
        
        uint32_t endvfn = ((uint32_t) addr - 1) / PAGE_SIZE + 1;
        uint32_t startvfn = ((uint32_t) curproc->p_brk - 1) / PAGE_SIZE + 1;

        if (endvfn > startvfn ){   
                dbg(DBG_PRINT, "(GRADING3B)\n");

	            vmarea_t *vmarea = vmmap_lookup(map, startvfn-1);
	            int isEmpty = vmmap_is_range_empty(map, startvfn, (endvfn - startvfn));
	                
	            if(isEmpty != 1) { 
		                dbg(DBG_PRINT,"(GRADING3E)\n");
		                return -ENOMEM;
	            }
	            
                    dbg(DBG_PRINT, "(GRADING3B)\n");
	            vmarea->vma_end = endvfn;
        }
  
        
        else if (endvfn < startvfn){
                dbg(DBG_PRINT,"(GRADING3E)\n");
            	vmmap_remove(map, endvfn, (startvfn - endvfn));
        }
        
        dbg(DBG_PRINT, "(GRADING3B)\n");
        curproc->p_brk = addr;
        *ret = addr;
        return 0;
}
