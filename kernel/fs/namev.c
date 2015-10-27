/******************************************************************************/
/* Important Spring 2015 CSCI 402 usage information:                          */
/*                                                                            */
/* This fils is part of CSCI 402 kernel programming assignments at USC.       */
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
 * specific lookup() function, but you may want to special case
 * "." and/or ".." here depnding on your implementation.
 *
 * If dir has no lookup(), return -ENOTDIR.
 *
 * Note: returns with the vnode refcount on *result incremented.
 */
int
lookup(vnode_t *dir, const char *name, size_t len, vnode_t **result)
{
        /*NOT_YET_IMPLEMENTED("VFS: lookup");*/

        KASSERT(NULL!=dir);
        dbg(DBG_PRINT,"(GRADING2A 2.a)\n");

        KASSERT(NULL!=name);
        dbg(DBG_PRINT,"(GRADING2A 2.a)\n");

        KASSERT(NULL!=result);
        dbg(DBG_PRINT,"(GRADING2A 2.a)\n");

        if(len>NAME_LEN)
        {
                return -ENAMETOOLONG;
        }
        if(len==0)
        {
                *result=dir;
                vref(*result);
                return 0;
        }
 
        if(S_ISDIR(dir->vn_mode)==0)
        {
                return -ENOTDIR;
        }

        char tempname[30];
        strncpy(tempname,name,len);
        if(strcmp(tempname,".")==0)
        {
                vref(dir);
                *result=dir;
                return 0;
        }
        int reval=(dir->vn_ops->lookup)(dir,tempname,len,result);

        if(reval<0)
        {
                return -ENOENT;
        }
        return 0;
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
        /*NOT_YET_IMPLEMENTED("VFS: dir_namev");*/
        KASSERT(NULL!=pathname);
        dbg(DBG_PRINT,"(GRADING2A 2.b)\n");

        KASSERT(NULL!=namelen);
        dbg(DBG_PRINT,"(GRADING2A 2.b)\n");

        KASSERT(NULL!=name);
        dbg(DBG_PRINT,"(GRADING2A 2.b)\n");

        KASSERT(NULL!=res_vnode);
        dbg(DBG_PRINT,"(GRADING2A 2.b)\n");

        vnode_t *begin_node=NULL;
        vnode_t *tempres=NULL;
        const char* start=pathname;
/*
        if(strlen(pathname)>MAXPATHLEN)
        {
                return -ENAMETOOLONG;
        }
*/
	if(strlen(pathname)==0)
        {
		return -EINVAL;
	}
	if(pathname[0]=='/')
        {
		begin_node = vfs_root_vn;
                vref(begin_node);
		start++;
	}
        else if(base==NULL)
        {
		begin_node = curproc->p_cwd;
                vref(begin_node);
	}
        else
        {
                begin_node=base;
                vref(begin_node);
        }

        const char* sep_token=NULL;
	while((sep_token=strchr(start, '/')))
        {
		KASSERT(begin_node != NULL);
                int res=lookup(begin_node, start,  sep_token-start, &tempres);
		if(res<0)
                {
                        vput(begin_node);
			return res;
		}

                KASSERT(NULL!=tempres);
                dbg(DBG_PRINT,"(GRADING2A 2.b)\n");

                vput(begin_node);
		begin_node = tempres;
                start = sep_token+1;
	}

	*namelen=strlen(start);
	if(*namelen > NAME_LEN) {
		vput(begin_node);
		return -ENAMETOOLONG;
	}
	*name = start;
	*res_vnode=begin_node;
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
        /*NOT_YET_IMPLEMENTED("VFS: open_namev");*/

        size_t namelen;
        const char *name=NULL;
        vnode_t *path_first_vnode=NULL;
        int reval1=dir_namev(pathname, &namelen, &name,base, &path_first_vnode);
        if(reval1<0)
        {
                return reval1;
        }
        int reval2=lookup(path_first_vnode,name,namelen,res_vnode);
        if(reval2<0)
        {
                if(reval2==-ENOTDIR)
                {
                        vput(path_first_vnode);
                        return reval2;
                }
                if(reval2==-ENAMETOOLONG)
                {
                        vput(path_first_vnode);
                        return reval2;
                }
                if((flag==(O_RDONLY|O_CREAT))||(flag==(O_WRONLY|O_CREAT))||(flag==(O_RDWR|O_CREAT)))
                {
                        KASSERT(NULL!=path_first_vnode->vn_ops->create);
                        dbg(DBG_PRINT,"(GRADING2A 2.c)\n");
                        
                        int reval3=(path_first_vnode->vn_ops->create)(path_first_vnode,name,namelen,res_vnode);

                        if(reval3<0)
                        {
                                vput(path_first_vnode);
                                return reval3;
                        }
                }
                else{
                        vput(path_first_vnode);
                        return -ENOENT;
                }
        }
        vput(path_first_vnode);
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
