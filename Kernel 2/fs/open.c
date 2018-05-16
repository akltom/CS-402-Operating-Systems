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

/*
 *  FILE: open.c
 *  AUTH: mcc | jal
 *  DESC:
 *  DATE: Mon Apr  6 19:27:49 1998
 */

#include "globals.h"
#include "errno.h"
#include "fs/fcntl.h"
#include "util/string.h"
#include "util/printf.h"
#include "fs/vfs.h"
#include "fs/vnode.h"
#include "fs/file.h"
#include "fs/vfs_syscall.h"
#include "fs/open.h"
#include "fs/stat.h"
#include "util/debug.h"

/* find empty index in p->p_files[] */
int
get_empty_fd(proc_t *p)
{
        int fd;

        for (fd = 0; fd < NFILES; fd++) {
                if (!p->p_files[fd])
                        return fd;
        }

        dbg(DBG_ERROR | DBG_VFS, "ERROR: get_empty_fd: out of file descriptors "
            "for pid %d\n", curproc->p_pid);
        return -EMFILE;
}

/*
 * There a number of steps to opening a file:
 *      1. Get the next empty file descriptor.
 *      2. Call fget to get a fresh file_t.
 *      3. Save the file_t in curproc's file descriptor table.
 *      4. Set file_t->f_mode to OR of FMODE_(READ|WRITE|APPEND) based on
 *         oflags, which can be O_RDONLY, O_WRONLY or O_RDWR, possibly OR'd with
 *         O_APPEND or O_CREAT.
 *      5. Use open_namev() to get the vnode for the file_t.
 *      6. Fill in the fields of the file_t.
 *      7. Return new fd.
 *
 * If anything goes wrong at any point (specifically if the call to open_namev
 * fails), be sure to remove the fd from curproc, fput the file_t and return an
 * error.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EINVAL
 *        oflags is not valid.
 *      o EMFILE
 *        The process already has the maximum number of files open.
 *      o ENOMEM
 *        Insufficient kernel memory was available.
 *      o ENAMETOOLONG
 *        A component of filename was too long.
 *      o ENOENT
 *        O_CREAT is not set and the named file does not exist.  Or, a
 *        directory component in pathname does not exist.
 *      o EISDIR
 *        pathname refers to a directory and the access requested involved
 *        writing (that is, O_WRONLY or O_RDWR is set).
 *      o ENXIO
 *        pathname refers to a device special file and no corresponding device
 *        exists.
 */

int
do_open(const char *filename, int oflags)
{       

        if(oflags != 0x000 && oflags != 0x001 && oflags != 0x002 && oflags != 0x100 && oflags != 0x101 && oflags != 0x102 &&
            oflags != 0x200 && oflags != 0x201 && oflags != 0x202 && oflags != 0x400 && oflags != 0x401 && oflags != 0x402) {
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return -EINVAL;
        }

        int fd = get_empty_fd(curproc);
        /* 
            if(fd == -EMFILE){
                dbg(DBG_PRINT, "(GRADING2D)\n");
                return -EMFILE;
        }*/

        file_t *f = fget(-1);
        /*if(f == NULL){
                dbg(DBG_PRINT, "(GRADING2D)\n");
                return -ENOMEM;
        }*/

        curproc->p_files[fd] = f;

        f->f_mode = 0;
        if((oflags & O_RDONLY) == O_RDONLY){
                dbg(DBG_PRINT, "(GRADING2B)\n");
                f->f_mode = FMODE_READ;
        }
        
        if((oflags & O_WRONLY) == O_WRONLY){
                dbg(DBG_PRINT, "(GRADING2B)\n");
                f->f_mode = FMODE_WRITE;
        }

        if((oflags & O_RDWR) == O_RDWR){
                dbg(DBG_PRINT, "(GRADING2B)\n");
                f->f_mode = (FMODE_READ | FMODE_WRITE);
        }
    
        if ((oflags & O_APPEND) == O_APPEND) {
                dbg(DBG_PRINT, "(GRADING2B)\n");
                f->f_mode |= FMODE_APPEND;
        }
        
        vnode_t *vno = NULL;

        int ret = 0;

        if((ret = open_namev(filename, oflags, &vno, NULL)) != 0){

                dbg(DBG_PRINT, "(GRADING2B)\n");
                fput(f);
                curproc->p_files[fd] = NULL;
                return ret;

        } else if(S_ISDIR(vno->vn_mode) && (((oflags & O_WRONLY) == O_WRONLY) || ((oflags & O_RDWR) == O_RDWR))){

                dbg(DBG_PRINT, "(GRADING2B)\n");
                vput(vno);
                fput(f);
                curproc->p_files[fd] = NULL;
                return -EISDIR;
        } else if(S_ISCHR(vno->vn_mode)){
             
                /*if(vno->vn_cdev != bytedev_lookup(vno->vn_devid)){
                        dbg(DBG_PRINT, "(GRADING2D)\n");
                        vput(vno);
                        fput(f);
                        curproc->p_files[fd] = NULL;
                        return -ENXIO;
                }*/
                dbg(DBG_PRINT, "(GRADING2B)\n");

        }
        /* 
        else if(S_ISBLK(vno->vn_mode)){
            
                if(vno->vn_bdev != blockdev_lookup(vno->vn_devid)){
                        dbg(DBG_PRINT, "(GRADING2E)\n");
                        vput(vno);
                        fput(f);
                        curproc->p_files[fd] = NULL;
                        return -ENXIO;
                }
                dbg(DBG_PRINT, "(GRADING2D)\n");
        }
        */
        
        if (strlen(filename) > 0 && filename[strlen(filename)-1] == '/' && !S_ISDIR(vno->vn_mode))
        {
                dbg(DBG_PRINT, "(GRADING2B)\n");
                vput(vno);
                fput(f);
                curproc->p_files[fd] = NULL;
                return -ENOTDIR;
        }
        
        dbg(DBG_PRINT, "(GRADING2B)\n");

        f->f_vnode = vno;
        f->f_pos = 0;

        return fd;
        
}