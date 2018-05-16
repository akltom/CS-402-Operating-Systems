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
 *  FILE: vfs_syscall.c
 *  AUTH: mcc | jal
 *  DESC:
 *  DATE: Wed Apr  8 02:46:19 1998
 *  $Id: vfs_syscall.c,v 1.13 2015/12/15 14:38:24 william Exp $
 */

#include "kernel.h"
#include "errno.h"
#include "globals.h"
#include "fs/vfs.h"
#include "fs/file.h"
#include "fs/vnode.h"
#include "fs/vfs_syscall.h"
#include "fs/open.h"
#include "fs/fcntl.h"
#include "fs/lseek.h"
#include "mm/kmalloc.h"
#include "util/string.h"
#include "util/printf.h"
#include "fs/stat.h"
#include "util/debug.h"

/* To read a file:
 *      o fget(fd)
 *      o call its virtual read fs_op
 *      o update f_pos
 *      o fput() it
 *      o return the number of bytes read, or an error
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EBADF
 *        fd is not a valid file descriptor or is not open for reading.
 *      o EISDIR
 *        fd refers to a directory.
 *
 * In all cases, be sure you do not leak file refcounts by returning before
 * you fput() a file that you fget()'ed.
 */
int
do_read(int fd, void *buf, size_t nbytes)
{
        /*NOT_YET_IMPLEMENTED("VFS: do_read");*/
        if (fd < 0 || fd >= NFILES)
        {
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return -EBADF;
        }

        file_t *ft = fget(fd);  
        if (ft == NULL)
        {
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return -EBADF;
        }

        if (!(ft->f_mode & FMODE_READ))
        {
                dbg(DBG_PRINT, "(GRADING2B)\n");
                fput(ft);
                return -EBADF;
        }

        if (S_ISDIR(ft->f_vnode->vn_mode))
        {
                dbg(DBG_PRINT, "(GRADING2B)\n");
                fput(ft);
                return -EISDIR;
        }

        dbg(DBG_PRINT, "(GRADING2B)\n");
        int count = ft->f_vnode->vn_ops->read(ft->f_vnode, ft->f_pos, buf, nbytes);
        ft->f_pos += count;
        /*
        if(ft->f_pos < 0) {
                ft->f_pos -= count;         
                fput(ft);
                return 0;        
        }
        */
        fput(ft);

        return count;
}

/* Very similar to do_read.  Check f_mode to be sure the file is writable.  If
 * f_mode & FMODE_APPEND, do_lseek() to the end of the file, call the write
 * fs_op, and fput the file.  As always, be mindful of refcount leaks.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EBADF
 *        fd is not a valid file descriptor or is not open for writing.
 */
int
do_write(int fd, const void *buf, size_t nbytes)
{
        /*NOT_YET_IMPLEMENTED("VFS: do_write");*/
        if (fd < 0 || fd >= NFILES)
        {
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return -EBADF;
        }

        file_t *ft = fget(fd);  
        if (ft == NULL)
        {
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return -EBADF;
        }

        if (!(ft->f_mode & FMODE_WRITE))
        {       
                dbg(DBG_PRINT, "(GRADING2B)\n"); 
                fput(ft);
                return -EBADF;
        }

        if (ft->f_mode & FMODE_APPEND)
        {
                dbg(DBG_PRINT, "(GRADING2B)\n");
                do_lseek(fd, 0, SEEK_END);
        }

        int count = ft->f_vnode->vn_ops->write(ft->f_vnode, ft->f_pos, buf, nbytes);
        ft->f_pos += count; 
        /*
        if(ft->f_pos < 0) {
                ft->f_pos -= count;         
                fput(ft);
                return 0;        
        }
        */
        KASSERT((S_ISCHR(ft->f_vnode->vn_mode)) || (S_ISBLK(ft->f_vnode->vn_mode)) || 
                ((S_ISREG(ft->f_vnode->vn_mode)) && (ft->f_pos <= ft->f_vnode->vn_len)));
        dbg(DBG_PRINT, "(GRADING2A 3.a)\n");
        fput(ft);

        return count;
}

/*
 * Zero curproc->p_files[fd], and fput() the file. Return 0 on success
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EBADF
 *        fd isn't a valid open file descriptor.
 */
int
do_close(int fd)
{
        /*         NOT_YET_IMPLEMENTED("VFS: do_close"); */
        if (fd < 0 || fd >= NFILES || curproc->p_files[fd] == NULL)
        {
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return -EBADF;
        }
        dbg(DBG_PRINT, "(GRADING2B)\n");
        
        fput(curproc->p_files[fd]);
        curproc->p_files[fd] = NULL;
        
        return 0;
}

/* To dup a file:
 *      o fget(fd) to up fd's refcount
 *      o get_empty_fd()
 *      o point the new fd to the same file_t* as the given fd
 *      o return the new file descriptor
 *
 * Don't fput() the fd unless something goes wrong.  Since we are creating
 * another reference to the file_t*, we want to up the refcount.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EBADF
 *        fd isn't an open file descriptor.
 *      o EMFILE
 *        The process already has the maximum number of file descriptors open
 *        and tried to open a new one.
 */
int
do_dup(int fd)
{
        /*NOT_YET_IMPLEMENTED("VFS: do_dup");*/
        if (fd < 0 || fd >= NFILES)
        {
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return -EBADF;
        }

        file_t *ft = fget(fd);  

        if ( ft == NULL)
        {
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return -EBADF;
        }

        int newfd = get_empty_fd(curproc);
        /*
        if (newfd == -EMFILE)
        {
                dbg(DBG_PRINT, "(GRADING2D)\n");
                fput(ft);
                return -EMFILE;
        }
        */
        dbg(DBG_PRINT, "(GRADING2B)\n");
        curproc->p_files[newfd] = ft;

        return newfd;
}

/* Same as do_dup, but insted of using get_empty_fd() to get the new fd,
 * they give it to us in 'nfd'.  If nfd is in use (and not the same as ofd)
 * do_close() it first.  Then return the new file descriptor.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EBADF
 *        ofd isn't an open file descriptor, or nfd is out of the allowed
 *        range for file descriptors.
 */
int
do_dup2(int ofd, int nfd)
{
        /*NOT_YET_IMPLEMENTED("VFS: do_dup2"); */

        if (ofd < 0 || ofd >= NFILES)
        {
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return -EBADF;
        }

        file_t *ft = fget(ofd);  

        if ( ft == NULL)
        {
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return -EBADF;
        }
        /*
        if (nfd >= NFILES || nfd < 0)
        {
                dbg(DBG_PRINT, "(GRADING2D)\n");
                fput(ft);
                return -EBADF;
        }
        */

        if (curproc->p_files[nfd] != NULL)
        {
                dbg(DBG_PRINT, "(GRADING2B)\n");
                do_close(nfd);
        }

        dbg(DBG_PRINT, "(GRADING2B)\n");
        curproc->p_files[nfd] = ft;

        return nfd;
}

/*
 * This routine creates a special file of the type specified by 'mode' at
 * the location specified by 'path'. 'mode' should be one of S_IFCHR or
 * S_IFBLK (you might note that mknod(2) normally allows one to create
 * regular files as well-- for simplicity this is not the case in Weenix).
 * 'devid', as you might expect, is the device identifier of the device
 * that the new special file should represent.
 *
 * You might use a combination of dir_namev, lookup, and the fs-specific
 * mknod (that is, the containing directory's 'mknod' vnode operation).
 * Return the result of the fs-specific mknod, or an error.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EINVAL
 *        mode requested creation of something other than a device special
 *        file.
 *      o EEXIST
 *        path already exists.
 *      o ENOENT
 *        A directory component in path does not exist.
 *      o ENOTDIR
 *        A component used as a directory in path is not, in fact, a directory.
 *      o ENAMETOOLONG
 *        A component of path was too long.
 */
int
do_mknod(const char *path, int mode, unsigned devid)
{
        /*NOT_YET_IMPLEMENTED("VFS: do_mknod");*/
        /*
        if (!S_ISBLK(mode) && !S_ISCHR(mode))
        {
                dbg(DBG_PRINT, "(GRADING2D)\n");
                return -EINVAL;
        } 
        */
        size_t len = 0;
        vnode_t *res_vnode = NULL;

        dir_namev(path, &len, &path, NULL, &res_vnode);
        /*
        if (errno < 0)
        {
                dbg(DBG_PRINT, "(GRADING2D)\n");
                return errno;
        }
        */
        
        vnode_t *exist = NULL;
        
        int errno = lookup(res_vnode, path, len, &exist);
        /*
        if (exist != NULL)
        {
                dbg(DBG_PRINT, "(GRADING2D)\n");
                vput(res_vnode);
                vput(exist);
                return -EEXIST;
        }        
        
        
        if (errno == -ENOTDIR)
        {
                dbg(DBG_PRINT, "(GRADING2D)\n");
                vput(res_vnode);
                return -ENOTDIR;
        }

        if (errno == -ENAMETOOLONG)
        {
                dbg(DBG_PRINT, "(GRADING2D)\n");
                vput(res_vnode);
                return -ENAMETOOLONG;
        } 
        */
        
        KASSERT(NULL != res_vnode->vn_ops->mknod);
        dbg(DBG_PRINT, "(GRADING2A 3.b)\n");
        
        int res = res_vnode->vn_ops->mknod(res_vnode, path, len, mode, (devid_t) devid);
        vput(res_vnode);

        return res;
        
}

/* Use dir_namev() to find the vnode of the dir we want to make the new
 * directory in.  Then use lookup() to make sure it doesn't already exist.
 * Finally call the dir's mkdir vn_ops. Return what it returns.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EEXIST
 *        path already exists.
 *      o ENOENT
 *        A directory component in path does not exist.
 *      o ENOTDIR
 *        A component used as a directory in path is not, in fact, a directory.
 *      o ENAMETOOLONG
 *        A component of path was too long.
 */
int
do_mkdir(const char *path)
{
        /* NOT_YET_IMPLEMENTED("VFS: do_mkdir");  */

        size_t len = 0;
        vnode_t *res_vnode = NULL;

        int errno = dir_namev(path, &len, &path, NULL, &res_vnode);

        if (errno < 0)
        {
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return errno;
        }

        vnode_t *exist = NULL;

        errno = lookup(res_vnode, path, len, &exist);
        if (exist != NULL)
        {
                dbg(DBG_PRINT, "(GRADING2B)\n");
                vput(res_vnode);
                vput(exist);
                return -EEXIST;
        }    
        
        if (errno == -ENOTDIR)
        {
                dbg(DBG_PRINT, "(GRADING2B)\n");
                vput(res_vnode);
                return -ENOTDIR;
        }
        /*
        if (errno == -ENAMETOOLONG)
        {
                dbg(DBG_PRINT, "(GRADING2D)\n");
                vput(res_vnode);
                return -ENAMETOOLONG;
        } 
        */
        KASSERT(NULL != res_vnode->vn_ops->mkdir);
        dbg(DBG_PRINT, "(GRADING2A 3.c)\n");
        int res = res_vnode->vn_ops->mkdir(res_vnode, path, len);
        vput(res_vnode);
        return res;
}

/* Use dir_namev() to find the vnode of the directory containing the dir to be
 * removed. Then call the containing dir's rmdir v_op.  The rmdir v_op will
 * return an error if the dir to be removed does not exist or is not empty, so
 * you don't need to worry about that here. Return the value of the v_op,
 * or an error.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EINVAL
 *        path has "." as its final component.
 *      o ENOTEMPTY
 *        path has ".." as its final component.
 *      o ENOENT
 *        A directory component in path does not exist.
 *      o ENOTDIR
 *        A component used as a directory in path is not, in fact, a directory.
 *      o ENAMETOOLONG
 *        A component of path was too long.
 */
int
do_rmdir(const char *path)
{
        /*NOT_YET_IMPLEMENTED("VFS: do_rmdir");*/

        size_t len = 0;
        vnode_t *res_vnode = NULL;
        int errno = dir_namev(path, &len, &path, NULL, &res_vnode);

        if (errno < 0)
        {
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return errno;
        }
        

        if (res_vnode->vn_ops->rmdir == NULL)
        {
                dbg(DBG_PRINT, "(GRADING2B)\n");
                vput(res_vnode);
                return -ENOTDIR;
        }
        
        if (len == 1 && name_match(path, ".", len))
        {
                dbg(DBG_PRINT, "(GRADING2B)\n");
                vput(res_vnode);
                return -EINVAL;
        }

        if (len == 2 && name_match(path, "..", len))
        {       
                dbg(DBG_PRINT, "(GRADING2B)\n");         
                vput(res_vnode);
                return -ENOTEMPTY;
        }

        KASSERT(NULL != res_vnode->vn_ops->rmdir);
        dbg(DBG_PRINT, "(GRADING2A 3.d)\n");
        dbg(DBG_PRINT, "(GRADING2B)\n"); /* depends */

        int res = res_vnode->vn_ops->rmdir(res_vnode, path, len);

        vput(res_vnode);

        return res;
}

/*
 * Same as do_rmdir, but for files.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EISDIR
 *        path refers to a directory.
 *      o ENOENT
 *        A component in path does not exist.
 *      o ENOTDIR
 *        A component used as a directory in path is not, in fact, a directory.
 *      o ENAMETOOLONG
 *        A component of path was too long.
 */
int
do_unlink(const char *path)
{
        /* NOT_YET_IMPLEMENTED("VFS: do_unlink");*/
        size_t len = 0;

        vnode_t *res_vnode = NULL;
        int errno = dir_namev(path, &len, &path, NULL, &res_vnode);
        /*
        if (errno < 0)
        {
                dbg(DBG_PRINT, "(GRADING2D)\n");
                return errno;
        }
        */
        vnode_t *node = NULL;

        errno = lookup(res_vnode, path, len, &node);

        if (errno < 0)
        {
                
                dbg(DBG_PRINT, "(GRADING2B)\n");vput(res_vnode);
                return errno;
        }

        if (S_ISDIR(node->vn_mode))
        {
                dbg(DBG_PRINT, "(GRADING2B)\n");
                vput(res_vnode);
                vput(node);
                return -EISDIR;
        }

        KASSERT(NULL != res_vnode->vn_ops->unlink);
        dbg(DBG_PRINT, "(GRADING2A 3.e)\n");
        dbg(DBG_PRINT, "(GRADING2B)\n"); /* depends */
        int res = res_vnode->vn_ops->unlink(res_vnode, path, len);

        vput(res_vnode);
        vput(node);

        return res;
}

/* To link:
 *      o open_namev(from)
 *      o dir_namev(to)
 *      o call the destination dir's (to) link vn_ops.
 *      o return the result of link, or an error
 *
 * Remember to vput the vnodes returned from open_namev and dir_namev.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EEXIST
 *        to already exists.
 *      o ENOENT
 *        A directory component in from or to does not exist.
 *      o ENOTDIR
 *        A component used as a directory in from or to is not, in fact, a
 *        directory.
 *      o ENAMETOOLONG
 *        A component of from or to was too long.
 *      o EISDIR
 *        from is a directory.
 */
int
do_link(const char *from, const char *to)
{
        NOT_YET_IMPLEMENTED("VFS: do_link");
        
        /*
        vnode_t *from_vnode = NULL;
        size_t len = 0;
        int errno = open_namev(from, O_CREAT, &from_vnode, NULL);
        
        if (errno < 0)
        {
                dbg(DBG_PRINT, "(GRADING2D)\n");
                return errno;
        }
        
        KASSERT(from_vnode != NULL);
        
        if (S_ISDIR(from_vnode->vn_mode))
        {
                dbg(DBG_PRINT, "(GRADING2D)\n");
                vput(from_vnode);
                return -EISDIR;
        }
        
        vnode_t *to_vnode = NULL;

        errno = dir_namev(to, &len, &to, NULL, &to_vnode);
        
        if (errno < 0)
        {
                dbg(DBG_PRINT, "(GRADING2D)\n");
                vput(from_vnode);
                return errno;
        }
        
        KASSERT(to_vnode != NULL);

        vnode_t *exist = NULL;
        errno = lookup(to_vnode, to, len, &exist);
        
        if (exist != NULL)
        {
                dbg(DBG_PRINT, "(GRADING2D)\n");
                vput(exist);
                vput(from_vnode);
                vput(to_vnode);
                return -EEXIST;
        }  

        if (errno < 0)
        {
                dbg(DBG_PRINT, "(GRADING2D)\n");
                vput(from_vnode);
                vput(to_vnode);
                return errno;
        }
        

        dbg(DBG_PRINT, "(GRADING2D)\n");
        int r = to_vnode->vn_ops->link(from_vnode, to_vnode, to, len);
        vput(from_vnode);
        vput(to_vnode);
        return r;
        */
        return 0;
}

/*      o link newname to oldname
 *      o unlink oldname
 *      o return the value of unlink, or an error
 *
 * Note that this does not provide the same behavior as the
 * Linux system call (if unlink fails then two links to the
 * file could exist).
 */
int
do_rename(const char *oldname, const char *newname)
{
        NOT_YET_IMPLEMENTED("VFS: do_rename");
        /*
        int errno = do_link(oldname, newname);

        if (errno != 0)
        {
                dbg(DBG_PRINT, "(GRADING2D)\n");
                return errno;
        }
        dbg(DBG_PRINT, "(GRADING2D)\n");
        
        return do_unlink(oldname);
        */
        return 0;
        
}

/* Make the named directory the current process's cwd (current working
 * directory).  Don't forget to down the refcount to the old cwd (vput()) and
 * up the refcount to the new cwd (open_namev() or vget()). Return 0 on
 * success.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o ENOENT
 *        path does not exist.
 *      o ENAMETOOLONG
 *        A component of path was too long.
 *      o ENOTDIR
 *        A component of path is not a directory.
 */
int
do_chdir(const char *path)
{
        /*NOT_YET_IMPLEMENTED("VFS: do_chdir");*/

        vnode_t *node = NULL;
        int errno = open_namev(path, 0, &node, NULL);

        if (node == NULL)
        {
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return -ENOENT;
        }
        /*
        if (errno < 0)
        {
                dbg(DBG_PRINT, "(GRADING2E)\n");
                return errno;
        }
        */

        if (!S_ISDIR(node->vn_mode))
        {
                dbg(DBG_PRINT, "(GRADING2B)\n");
                vput(node);
                return -ENOTDIR;
        }

        dbg(DBG_PRINT, "(GRADING2B)\n");
        vput(curproc->p_cwd);        
        curproc->p_cwd = node;

        return 0;
}

/* Call the readdir fs_op on the given fd, filling in the given dirent_t*.
 * If the readdir fs_op is successful, it will return a positive value which
 * is the number of bytes copied to the dirent_t.  You need to increment the
 * file_t's f_pos by this amount.  As always, be aware of refcounts, check
 * the return value of the fget and the virtual function, and be sure the
 * virtual function exists (is not null) before calling it.
 *
 * Return either 0 or sizeof(dirent_t), or -errno.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EBADF
 *        Invalid file descriptor fd.
 *      o ENOTDIR
 *        File descriptor does not refer to a directory.
 */
int
do_getdent(int fd, struct dirent *dirp)
{
        /*NOT_YET_IMPLEMENTED("VFS: do_getdent");*/
        if (fd < 0 || fd >= NFILES)
        {
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return -EBADF;
        }

        file_t *ft = fget(fd);  
        if (ft == NULL)
        {
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return -EBADF;
        }

        if (!S_ISDIR(ft->f_vnode->vn_mode) || ft->f_vnode->vn_ops->readdir == NULL)
        {
                dbg(DBG_PRINT, "(GRADING2B)\n");
                fput(ft);
                return -ENOTDIR;
        } 

        int off = ft->f_vnode->vn_ops->readdir(ft->f_vnode, ft->f_pos, dirp);
        ft->f_pos += off;
        fput(ft);

        if (off == 0)
        {
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return 0;
        }

        dbg(DBG_PRINT, "(GRADING2B)\n");
        return sizeof(*dirp);
}

/*
 * Modify f_pos according to offset and whence.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EBADF
 *        fd is not an open file descriptor.
 *      o EINVAL
 *        whence is not one of SEEK_SET, SEEK_CUR, SEEK_END; or the resulting
 *        file offset would be negative.
 */
int
do_lseek(int fd, int offset, int whence)
{
        /*NOT_YET_IMPLEMENTED("VFS: do_lseek");*/
        
        if (fd < 0 || fd >= NFILES)
        {
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return -EBADF;
        }

        file_t *ft = fget(fd);  

        if (ft == NULL)
        {
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return -EBADF;
        }

        int pre = ft->f_pos;

        if (whence == SEEK_SET)
        {
                dbg(DBG_PRINT, "(GRADING2B)\n");
                ft->f_pos = offset;
        }

        else if (whence == SEEK_CUR)
        {
                dbg(DBG_PRINT, "(GRADING2B)\n");
                ft->f_pos += offset;
        }

        else if (whence == SEEK_END)
        {
                dbg(DBG_PRINT, "(GRADING2B)\n");
                ft->f_pos = ft->f_vnode->vn_len + offset;
        }

        else {
                dbg(DBG_PRINT, "(GRADING2B)\n");
                fput(ft);
                return -EINVAL;
        }

        if (ft->f_pos < 0)
        {
                dbg(DBG_PRINT, "(GRADING2B)\n");
                ft->f_pos = pre;
                fput(ft);
                return -EINVAL;
        }
        dbg(DBG_PRINT, "(GRADING2B)\n");
        int res = ft->f_pos;
        fput(ft);
        return res;
}

/*
 * Find the vnode associated with the path, and call the stat() vnode operation.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o ENOENT
 *        A component of path does not exist.
 *      o ENOTDIR
 *        A component of the path prefix of path is not a directory.
 *      o ENAMETOOLONG
 *        A component of path was too long.
 */

int
do_stat(const char *path, struct stat *buf)
{
        /*NOT_YET_IMPLEMENTED("VFS: do_stat");*/

        if (path[0] == '\0')
        {
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return -EINVAL;
        }

        vnode_t *node = NULL;
        int errno = open_namev(path, 0, &node, NULL);
        
        if (errno < 0)
        {
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return errno;
        }

        KASSERT(node->vn_ops->stat);
        dbg(DBG_PRINT, "(GRADING2A 3.f)\n");
        dbg(DBG_PRINT, "(GRADING2D)\n"); /* depends */

        int ret = node->vn_ops->stat(node, buf);

        vput(node);

        return ret;
}

#ifdef __MOUNTING__
/*
 * Implementing this function is not required and strongly discouraged unless
 * you are absolutely sure your Weenix is perfect.
 *
 * This is the syscall entry point into vfs for mounting. You will need to
 * create the fs_t struct and populate its fs_dev and fs_type fields before
 * calling vfs's mountfunc(). mountfunc() will use the fields you populated
 * in order to determine which underlying filesystem's mount function should
 * be run, then it will finish setting up the fs_t struct. At this point you
 * have a fully functioning file system, however it is not mounted on the
 * virtual file system, you will need to call vfs_mount to do this.
 *
 * There are lots of things which can go wrong here. Make sure you have good
 * error handling. Remember the fs_dev and fs_type buffers have limited size
 * so you should not write arbitrary length strings to them.
 */
int
do_mount(const char *source, const char *target, const char *type)
{
        NOT_YET_IMPLEMENTED("MOUNTING: do_mount");
        return -EINVAL;
}

/*
 * Implementing this function is not required and strongly discouraged unless
 * you are absolutley sure your Weenix is perfect.
 *
 * This function delegates all of the real work to vfs_umount. You should not worry
 * about freeing the fs_t struct here, that is done in vfs_umount. All this function
 * does is figure out which file system to pass to vfs_umount and do good error
 * checking.
 */
int
do_umount(const char *target)
{
        NOT_YET_IMPLEMENTED("MOUNTING: do_umount");
        return -EINVAL;
}
#endif
