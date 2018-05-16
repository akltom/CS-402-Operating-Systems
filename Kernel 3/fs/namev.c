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
#include "globals.h"
#include "types.h"
#include "errno.h"

#include "util/string.h"
#include "util/printf.h"
#include "util/debug.h"

#include "fs/dirent.h"
#include "fs/fcntl.h"
#include "fs/stat.h"
#include "fs/vfs.h"
#include "fs/vnode.h"

/* This takes a base 'dir', a 'name', its 'len', and a result vnode.
 * Most of the work should be done by the vnode's implementation
 * specific lookup() function.
 *
 * If dir has no lookup(), return -ENOTDIR.
 *
 * Note: returns with the vnode refcount on *result incremented.
 */
int
lookup(vnode_t *dir, const char *name, size_t len, vnode_t **result)
{
         /*NOT_YET_IMPLEMENTED("VFS: lookup");*/
         KASSERT(NULL != dir);
         dbg(DBG_PRINT, "(GRADING2A 2.a)\n");
         KASSERT(NULL != name);
         dbg(DBG_PRINT, "(GRADING2A 2.a)\n");
         KASSERT(NULL != result);
         dbg(DBG_PRINT, "(GRADING2A 2.a)\n");

         /*Check if dir has no lookup*/

         if (len == 0)
         {
                 /*dbg(DBG_TEST, "--------------------------323\n");*/
                dbg(DBG_PRINT, "(GRADING2B)\n");
                *result = dir;
                vref(*result);
                return 0;
         }

         if (dir->vn_ops->lookup == NULL) {
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return -ENOTDIR;
         }

         int ret = dir->vn_ops->lookup(dir,name,len,result);
         dbg(DBG_PRINT, "(GRADING2B)\n");

         return ret;
 }


/* When successful this function returns data in the following "out"-arguments:
 *  o res_vnode: the vnode of the parent directory of "name"
 *  o name: the `basename' (the element of the pathname)
 *  o namelen: the length of the basename
 *
 * For example: dir_namev("/s5fs/bin/ls", &namelen, &name, NULL,
 * &res_vnode) would put 2 in namelen, "ls" in name, and a pointer to the
 * vnode corresponding to "/s5fs/bin" in res_vnode.
 *
 * The "base" argument defines where we start resolving the path from:
 * A base value of NULL means to use the process's current working directory,
 * curproc->p_cwd.  If pathname[0] == '/', ignore base and start with
 * vfs_root_vn.  dir_namev() should call lookup() to take care of resolving each
 * piece of the pathname.
 *
 * Note: A successful call to this causes vnode refcount on *res_vnode to
 * be incremented.
 */
int
dir_namev(const char *pathname, size_t *namelen, const char **name,
          vnode_t *base, vnode_t **res_vnode)
{
        KASSERT(NULL != pathname);
        dbg(DBG_PRINT, "(GRADING2A 2.b)\n");

        KASSERT(NULL != namelen);
        dbg(DBG_PRINT, "(GRADING2A 2.b)\n");

        KASSERT(NULL != name);
        dbg(DBG_PRINT, "(GRADING2A 2.b)\n");

        KASSERT(NULL != res_vnode);
        dbg(DBG_PRINT, "(GRADING2A 2.b)\n");

        vnode_t *currBase;
        if(!strcmp(pathname,"")){
            dbg(DBG_PRINT, "(GRADING2B)\n");
            return -EINVAL;
        }
        if(*pathname == '/'){
                dbg(DBG_PRINT, "(GRADING2B)\n");
                currBase = vfs_root_vn;
                /*
                if (strlen(pathname) == 1)
                {
                        *namelen = 0;
                        *name = pathname + 1;
                        *res_vnode = currBase;
                        return 0;
                }
                */

        } else {
                if(base == NULL){
                        dbg(DBG_PRINT, "(GRADING2B)\n");
                        currBase = curproc->p_cwd;
                } else {
                        dbg(DBG_PRINT, "(GRADING2B)\n");
                        currBase = base;
                }
        }
        KASSERT(NULL != currBase);
        dbg(DBG_PRINT, "(GRADING2A 2.b)\n");

        vref(currBase);
        vnode_t *nextDir = NULL;

        int i = 0,index = 0, result = 0;
        int start = 0, anchor = 0, file_len = 0;

        int k = strlen(pathname)-1;

        while(k>=0) {
                if (pathname[k] !='/')
                {
                        dbg(DBG_PRINT, "(GRADING2B)\n");
                        break;
                }
                dbg(DBG_PRINT, "(GRADING2B)\n");
                --k;
        }

        while(pathname[i] != '\0' && i<= k){
                if(pathname[start] != '/'){
                        dbg(DBG_PRINT, "(GRADING2B)\n");

                        start++;
                        index = 1;
                        file_len = start - anchor;
                } else {
                        int dir_len = start - anchor;
                        if(dir_len > NAME_LEN) {
                                dbg(DBG_PRINT, "(GRADING2B)\n");
                                vput(currBase);
                                return -ENAMETOOLONG;
                        }

                        if(index == 1){
                                result = lookup(currBase, pathname + anchor, dir_len, &nextDir);
                                if(result != 0){
                                        dbg(DBG_PRINT, "(GRADING2B)\n");
                                        vput(currBase);
                                        return result;
                                }
                                index = 0;
                                dbg(DBG_PRINT, "(GRADING2B)\n");
                                KASSERT(NULL != nextDir);
                                dbg(DBG_PRINT, "(GRADING2A 2.b)\n");
                                vput(currBase);
                                currBase = nextDir;
                        }

                        dbg(DBG_PRINT, "(GRADING2B)\n");
                        start++;
                        anchor = start;
                }

                dbg(DBG_PRINT, "(GRADING2B)\n");
                i++;
        }

        if(file_len > NAME_LEN){
                dbg(DBG_PRINT, "(GRADING2B)\n");
                vput(currBase);
                return -ENAMETOOLONG;
        }

        dbg(DBG_PRINT, "(GRADING2B)\n");
        *namelen = file_len;
        *name = pathname + anchor;
        *res_vnode = currBase;

        return 0;
}

/* This returns in res_vnode the vnode requested by the other parameters.
 * It makes use of dir_namev and lookup to find the specified vnode (if it
 * exists).  flag is right out of the parameters to open(2); see
 * <weenix/fcntl.h>.  If the O_CREAT flag is specified, and the file does
 * not exist call create() in the parent directory vnode.
 *
 * Note: Increments vnode refcount on *res_vnode.
 */
int
open_namev(const char *pathname, int flag, vnode_t **res_vnode, vnode_t *base)
{
        /* Initialize variables*/
        size_t namelen = 0;
        const char *name = NULL;
        vnode_t *res_vnode_dir; /* vnode for dir name v, used to be vn*/

        int dir_retval = dir_namev(pathname, &namelen, &name, base, &res_vnode_dir);
        if (dir_retval != 0){
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return dir_retval;
        }

        int retval = lookup(res_vnode_dir, name, namelen, res_vnode);
        if(retval != 0 ) {
                if(flag & O_CREAT && retval == -ENOENT){
                        KASSERT(NULL != res_vnode_dir->vn_ops->create);
                        dbg(DBG_PRINT, "(GRADING2A 2.c)\n");
                        dbg(DBG_PRINT, "(GRADING2B)\n");
                    
                        retval = res_vnode_dir->vn_ops->create(res_vnode_dir, name, namelen, res_vnode);
                        vput(res_vnode_dir);  /* free the vnode*/
                        return retval;
                }
             
                dbg(DBG_PRINT, "(GRADING2B)\n");      
                vput(res_vnode_dir);
                return retval;
        }
        
        dbg(DBG_PRINT, "(GRADING2B)\n");
        vput(res_vnode_dir);
        return 0;
}

#ifdef __GETCWD__
/* Finds the name of 'entry' in the directory 'dir'. The name is writen
 * to the given buffer. On success 0 is returned. If 'dir' does not
 * contain 'entry' then -ENOENT is returned. If the given buffer cannot
 * hold the result then it is filled with as many characters as possible
 * and a null terminator, -ERANGE is returned.
 *
 * Files can be uniquely identified within a file system by their
 * inode numbers. */
int
lookup_name(vnode_t *dir, vnode_t *entry, char *buf, size_t size)
{
        NOT_YET_IMPLEMENTED("GETCWD: lookup_name");
        return -ENOENT;
}


/* Used to find the absolute path of the directory 'dir'. Since
 * directories cannot have more than one link there is always
 * a unique solution. The path is writen to the given buffer.
 * On success 0 is returned. On error this function returns a
 * negative error code. See the man page for getcwd(3) for
 * possible errors. Even if an error code is returned the buffer
 * will be filled with a valid string which has some partial
 * information about the wanted path. */
ssize_t
lookup_dirpath(vnode_t *dir, char *buf, size_t osize)
{
        NOT_YET_IMPLEMENTED("GETCWD: lookup_dirpath");

        return -ENOENT;
}
#endif /* __GETCWD__ */
