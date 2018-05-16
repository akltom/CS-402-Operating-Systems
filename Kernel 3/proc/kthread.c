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

#include "config.h"
#include "globals.h"

#include "errno.h"

#include "util/init.h"
#include "util/debug.h"
#include "util/list.h"
#include "util/string.h"

#include "proc/kthread.h"
#include "proc/proc.h"
#include "proc/sched.h"

#include "mm/slab.h"
#include "mm/page.h"

kthread_t *curthr; /* global */
static slab_allocator_t *kthread_allocator = NULL;

#ifdef __MTP__
/* Stuff for the reaper daemon, which cleans up dead detached threads */
static proc_t *reapd = NULL;
static kthread_t *reapd_thr = NULL;
static ktqueue_t reapd_waitq;
static list_t kthread_reapd_deadlist; /* Threads to be cleaned */

static void *kthread_reapd_run(int arg1, void *arg2);
#endif

void
kthread_init()
{
        kthread_allocator = slab_allocator_create("kthread", sizeof(kthread_t));
        KASSERT(NULL != kthread_allocator);
}

/**
 * Allocates a new kernel stack.
 *
 * @return a newly allocated stack, or NULL if there is not enough
 * memory available
 */
static char *
alloc_stack(void)
{
        /* extra page for "magic" data */
        char *kstack;
        int npages = 1 + (DEFAULT_STACK_SIZE >> PAGE_SHIFT);
        kstack = (char *)page_alloc_n(npages);

        return kstack;
}

/**
 * Frees a stack allocated with alloc_stack.
 *
 * @param stack the stack to free
 */
static void
free_stack(char *stack)
{
        page_free_n(stack, 1 + (DEFAULT_STACK_SIZE >> PAGE_SHIFT));
}

void
kthread_destroy(kthread_t *t)
{
        KASSERT(t && t->kt_kstack);
        free_stack(t->kt_kstack);
        if (list_link_is_linked(&t->kt_plink))
                list_remove(&t->kt_plink);

        slab_obj_free(kthread_allocator, t);
}

/*
 * Allocate a new stack with the alloc_stack function. The size of the
 * stack is DEFAULT_STACK_SIZE.
 *
 * Don't forget to initialize the thread context with the
 * context_setup function. The context should have the same pagetable
 * pointer as the process.
 */
kthread_t *
kthread_create(struct proc *p, kthread_func_t func, long arg1, void *arg2)
{
        /*
        NOT_YET_IMPLEMENTED("PROCS: kthread_create"); */
        KASSERT(NULL != p);
        dbg(DBG_PRINT, "(GRADING1A 3.a)\n");

        KASSERT(kthread_allocator != NULL);

        kthread_t *thisKThread = (kthread_t*)slab_obj_alloc(kthread_allocator);
        /* allocate memory for this k thread */

        KASSERT(NULL != thisKThread);

        memset(thisKThread, 0, sizeof(kthread_t));    /*initilize*/

        thisKThread->kt_kstack = alloc_stack(); /* use alloc-stack function that we defined earlier above */
        KASSERT(NULL != thisKThread->kt_kstack);

        thisKThread->kt_retval = (void *)0;
        thisKThread->kt_errno = 0;

        thisKThread->kt_proc = p;
        KASSERT(NULL != thisKThread->kt_proc);

        thisKThread->kt_cancelled = 0;
        thisKThread->kt_wchan = NULL;

        thisKThread->kt_state = KT_RUN;  /* 1, from the thread.h */

        list_link_init(&thisKThread->kt_qlink);/* initialize the thread's links   */
        list_link_init(&thisKThread->kt_plink);

        list_insert_tail(&p->p_threads, &thisKThread->kt_plink);/* connect the list together */
        context_setup(&thisKThread->kt_ctx, func, arg1, arg2, thisKThread->kt_kstack, DEFAULT_STACK_SIZE, p->p_pagedir); /* found in context.h */
        return thisKThread;
}

/*
 * If the thread to be cancelled is the current thread, this is
 * equivalent to calling kthread_exit. Otherwise, the thread is
 * sleeping and we need to set the cancelled and retval fields of the
 * thread.
 *
 * If the thread's sleep is cancellable, cancelling the thread should
 * wake it up from sleep.
 *
 * If the thread's sleep is not cancellable, we do nothing else here.
 */
void
kthread_cancel(kthread_t *kthr, void *retval)
{
        KASSERT(NULL != kthr);
        dbg(DBG_PRINT, "(GRADING1A 3.b)\n");
        dbg(DBG_PRINT, "(GRADING1C)\n"); /* depends */

        /* NOT_YET_IMPLEMENTED("PROCS: kthread_cancel"); */
        if (kthr == curthr) {
                dbg(DBG_PRINT, "(GRADING1C 1)\n");
                return kthread_exit(retval);
        }

        kthr->kt_retval = retval;/* set the retival fields of the thread */
        kthr->kt_cancelled = 1;
        if (kthr->kt_state == KT_SLEEP_CANCELLABLE) { /*  define in kthred.h */
                dbg(DBG_PRINT, "(GRADING1C 1)\n");
                sched_cancel(kthr);   /* set the thread to be cancelled */

        }
        dbg(DBG_PRINT, "(GRADING1C 1)\n");
}

/*
 * You need to set the thread's retval field, set its state to
 * KT_EXITED, and alert the current process that a thread is exiting
 * via proc_thread_exited.
 *
 * It may seem unneccessary to push the work of cleaning up the thread
 * over to the process. However, if you implement MTP, a thread
 * exiting does not necessarily mean that the process needs to be
 * cleaned up.
 */
void
kthread_exit(void *retval)
{

        KASSERT(!curthr->kt_wchan); /* curthr should not be in any queue */
        dbg(DBG_PRINT, "(GRADING1A 3.c)\n");

        KASSERT(!curthr->kt_qlink.l_next && !curthr->kt_qlink.l_prev); /* queue should be empty */
        dbg(DBG_PRINT, "(GRADING1A 3.c)\n");

        KASSERT(curthr->kt_proc == curproc);
        dbg(DBG_PRINT, "(GRADING1A 3.c)\n");
        /*
        NOT_YET_IMPLEMENTED("PROCS: kthread_exit");
        */
        curthr->kt_retval = retval;
        curthr->kt_state = KT_EXITED;
        proc_thread_exited(retval);
}

/*
 * The new thread will need its own context and stack. Think carefully
 * about which fields should be copied and which fields should be
 * freshly initialized.
 *
 * You do not need to worry about this until VM.
 */
kthread_t *
kthread_clone(kthread_t *thr)
{


        KASSERT(KT_RUN == thr->kt_state);
        dbg(DBG_PRINT, "(GRADING3A 8.a)\n");
        
        dbg(DBG_PRINT, "(GRADING3B)\n");
        kthread_t *newthr = (kthread_t*) slab_obj_alloc(kthread_allocator);

        newthr->kt_kstack = alloc_stack();
        context_setup(&(newthr->kt_ctx), NULL, 0, NULL, newthr->kt_kstack, DEFAULT_STACK_SIZE, thr->kt_proc->p_pagedir);
        newthr->kt_retval = thr->kt_retval;
        newthr->kt_errno = thr->kt_errno;
        newthr->kt_proc = NULL;
        newthr->kt_cancelled = thr->kt_cancelled;
        newthr->kt_wchan = thr->kt_wchan;
        newthr->kt_state = thr->kt_state;
        /*not included in the self-check*/
        /*
        if(newthr->kt_wchan != NULL) {
                list_insert_head(&(newthr->kt_wchan->tq_list), &(newthr->kt_qlink));
                newthr->kt_wchan->tq_size++;
        }
        */
        list_link_init(&newthr->kt_qlink);
        list_link_init(&newthr->kt_plink);

        KASSERT(KT_RUN == newthr->kt_state);
        dbg(DBG_PRINT, "(GRADING3B)\n");
        

        return newthr;
}

/*
 * The following functions will be useful if you choose to implement
 * multiple kernel threads per process. This is strongly discouraged
 * unless your weenix is perfect.
 */
#ifdef __MTP__
int
kthread_detach(kthread_t *kthr)
{
        NOT_YET_IMPLEMENTED("MTP: kthread_detach");
        return 0;
}

int
kthread_join(kthread_t *kthr, void **retval)
{
        NOT_YET_IMPLEMENTED("MTP: kthread_join");
        return 0;
}

/* ------------------------------------------------------------------ */
/* -------------------------- REAPER DAEMON ------------------------- */
/* ------------------------------------------------------------------ */
static __attribute__((unused)) void
kthread_reapd_init()
{
        NOT_YET_IMPLEMENTED("MTP: kthread_reapd_init");
}
init_func(kthread_reapd_init);
init_depends(sched_init);

void
kthread_reapd_shutdown()
{
        NOT_YET_IMPLEMENTED("MTP: kthread_reapd_shutdown");
}

static void *
kthread_reapd_run(int arg1, void *arg2)
{
        NOT_YET_IMPLEMENTED("MTP: kthread_reapd_run");
        return (void *) 0;
}
#endif
