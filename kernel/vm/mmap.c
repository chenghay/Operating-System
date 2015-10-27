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
      
       if(addr!=NULL)
        {
                if((uintptr_t)addr<USER_MEM_LOW||(uintptr_t)addr>=USER_MEM_HIGH)
                {
                        return -EINVAL;
                }
                if(((uintptr_t)addr+len)<USER_MEM_LOW||((uintptr_t)addr+len)>=USER_MEM_HIGH)
                {
                        return -EINVAL;
                }
        }
        if(((flags & MAP_SHARED)!=MAP_SHARED)&&((flags & MAP_PRIVATE)!=MAP_PRIVATE)&&((flags & MAP_FIXED)!=MAP_FIXED)&&((flags & MAP_ANON)!=MAP_ANON))
        {
                return -EINVAL;
        }

        if(!PAGE_ALIGNED(off))
        {
                return -EINVAL;
        }
        if(len==0)
        {
                return -EINVAL;
        }
        if(addr==NULL && (flags&MAP_FIXED)==MAP_FIXED)
        {
                return -EINVAL;
        }

        if((addr==NULL&&(flags&MAP_FIXED)))
        {
                return -EINVAL;
        }
      
	


       
        vnode_t *vnd=NULL;
	vmarea_t *tempvm=NULL;
	 file_t *file1=NULL;
	if((flags & MAP_ANON)==0)
        {
		if(fd<0||fd>=NFILES)
                {
                        return -EBADF;
                }
                file_t *file1=fget(fd);
                if(file1==NULL)
                {
                        return -EBADF;
                }
                vnd=file1->f_vnode;

                if(flags&MAP_SHARED && prot&PROT_WRITE && (file1->f_mode&(FMODE_READ|FMODE_WRITE))!=(FMODE_READ|FMODE_WRITE))
                {
			
                        return -EACCES;
                }

                if(flags&MAP_PRIVATE && (file1->f_mode&FMODE_READ)!=FMODE_READ)
                {
                        return -EACCES;
                }
                uint32_t startpage;
                if(!(flags&MAP_FIXED))
                {
                        startpage=0;
                }
                else
                {
                        startpage=ADDR_TO_PN(addr);
                }
                uint32_t reslen=len/PAGE_SIZE;
                if(len%PAGE_SIZE!=0)
                {
                        reslen++;
                }
                int res=vmmap_map(curproc->p_vmmap,vnd,startpage,reslen,prot,flags,off,VMMAP_DIR_HILO,&tempvm);
                if(res<0)
                {
                        return -1;
                }
                *ret=PN_TO_ADDR(tempvm->vma_start); 
                tlb_flush_all();
                KASSERT(NULL != curproc->p_pagedir);
                dbg(DBG_PRINT, "(GRADING3A 2.a)\n");
                return 0; 
        }        
        uint32_t startpage;
        if(!(flags&MAP_FIXED))
        {
                startpage=0;
        }
        else
        {
                startpage=ADDR_TO_PN(addr);
        }
        uint32_t reslen=len/PAGE_SIZE;
        if(len%PAGE_SIZE!=0)
        {
                reslen++;
        }
        int res=vmmap_map(curproc->p_vmmap,vnd,startpage,reslen,prot,flags,off,VMMAP_DIR_HILO,&tempvm);
        if(res<0)
        {
                return -1;
        }
        *ret=PN_TO_ADDR(tempvm->vma_start); 
        tlb_flush_all();
        KASSERT(NULL != curproc->p_pagedir);
        dbg(DBG_PRINT, "(GRADING3A 2.a)\n");
	
        return 0;   
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
        /*NOT_YET_IMPLEMENTED("VM: do_munmap");*/
        if((uintptr_t)addr<USER_MEM_LOW||(uintptr_t)addr>=USER_MEM_HIGH)
        {
                 return -EINVAL;
        }
        if(((uintptr_t)addr+len)<USER_MEM_LOW||((uintptr_t)addr+len)>=USER_MEM_HIGH)
        {
                 return -EINVAL;
        } 
        if(!PAGE_ALIGNED(addr))
        {
                return -EINVAL;
        }
        if(((uintptr_t)addr)+len<=(uintptr_t)addr)
        {
                return -EINVAL;
        }

        uint32_t startpage=ADDR_TO_PN(addr);
        uint32_t reslen=len/PAGE_SIZE;
        if(len%PAGE_SIZE!=0)
        {
                reslen++;
        }
        int res=vmmap_remove(curproc->p_vmmap,startpage,reslen);
        tlb_flush_all();/**********do not forget to clear TLB**************/
        pt_unmap_range(curproc->p_pagedir,(uintptr_t)addr,(uintptr_t)PN_TO_ADDR(startpage+reslen));/*not automatically*/
        KASSERT(NULL != curproc->p_pagedir);
        dbg(DBG_PRINT, "(GRADING3A 2.b)\n");
        if(res<0)
        {
                return res;
        }
        return 0;
}

