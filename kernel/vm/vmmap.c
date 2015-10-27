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
        vmmap_t* new_vmmap_t=(vmmap_t*)slab_obj_alloc(vmmap_allocator);
        if(new_vmmap_t!=NULL)
        {
                new_vmmap_t->vmm_proc=NULL;
                list_init(&new_vmmap_t->vmm_list);
                return new_vmmap_t;
        }
        else{
                return NULL;
        }
}

/* Removes all vmareas from the address space and frees the
 * vmmap struct. */
void
vmmap_destroy(vmmap_t *map)
{
        /*NOT_YET_IMPLEMENTED("VM: vmmap_destroy");*/
        KASSERT(NULL != map);
        dbg(DBG_PRINT, "(GRADING3A 3.a)\n");
        if(list_empty(&(map->vmm_list))==0)
        {
                vmarea_t* temp_vmarea=NULL;
                list_iterate_begin(&(map->vmm_list),temp_vmarea,vmarea_t,vma_plink)
                {
                        if(temp_vmarea->vma_obj!=NULL)
                        {
                                temp_vmarea->vma_obj->mmo_ops->put(temp_vmarea->vma_obj);
                                list_remove(&(temp_vmarea->vma_plink));
                                list_remove(&(temp_vmarea->vma_olink));
                                vmarea_free(temp_vmarea);
                        }
                        else{
                                list_remove(&(temp_vmarea->vma_plink));
                                vmarea_free(temp_vmarea);
                        }
                }list_iterate_end();
             
                map->vmm_proc=NULL;
                slab_obj_free(vmmap_allocator,map);
        }
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
        dbg(DBG_PRINT, "(GRADING3A 3.b)\n");
        /*KASSERT(NULL == newvma->vma_vmmap);
        dbg(DBG_PRINT, "(GRADING3A 3.b)\n");*/
        KASSERT(newvma->vma_start < newvma->vma_end);
        dbg(DBG_PRINT, "(GRADING3A 3.b)\n");
        KASSERT(ADDR_TO_PN(USER_MEM_LOW) <= newvma->vma_start && ADDR_TO_PN(USER_MEM_HIGH) >= newvma->vma_end);
        dbg(DBG_PRINT, "(GRADING3A 3.b)\n");

        if(list_empty(&(map->vmm_list))==1)
        {
                newvma->vma_vmmap=map;
                list_insert_head(&(map->vmm_list),&(newvma->vma_plink));
                return;
        }
        vmarea_t* temp_vmarea=NULL;/****************how to avoid overlap when inserting into a sorted link******************/
        list_iterate_begin(&(map->vmm_list),temp_vmarea,vmarea_t,vma_plink)
        {
                if(newvma->vma_start<temp_vmarea->vma_start)/***********compare head or tail???????*************/
                {
                        list_insert_before(&(temp_vmarea->vma_plink), &(newvma->vma_plink));
                        newvma->vma_vmmap=map;
                        return;
                }
        }list_iterate_end();
        list_insert_tail(&(map->vmm_list),&(newvma->vma_plink));
        newvma->vma_vmmap=map;
        return;
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
        KASSERT(NULL != map);
        dbg(DBG_PRINT, "(GRADING3A 3.c)\n");
        KASSERT(0 < npages);
        dbg(DBG_PRINT, "(GRADING3A 3.c)\n");

        if(list_empty(&(map->vmm_list))==1)
        {
                uint32_t content=ADDR_TO_PN(USER_MEM_HIGH)-ADDR_TO_PN(USER_MEM_LOW);
                if(npages>content)
                {
                        return -1;
                }
                if(dir==VMMAP_DIR_LOHI)
                {
                        return ADDR_TO_PN(USER_MEM_LOW);
                }
                if(dir==VMMAP_DIR_HILO)
                {
                        return ADDR_TO_PN(USER_MEM_HIGH)-npages;/*[  ) no need to add 1???*/       
                }
        }
        else{
                /*the addrss space is not empty!!!!!!*/
                if(dir==VMMAP_DIR_LOHI)
                {
                        vmarea_t* temp_vmarea=NULL;
                        uint32_t begin=ADDR_TO_PN(USER_MEM_LOW);
                        uint32_t end;
                        list_iterate_begin(&(map->vmm_list),temp_vmarea,vmarea_t,vma_plink)
                        {
                                end=temp_vmarea->vma_start;
                                if(end-begin>=npages)
                                {
                                        return begin;
                                }
                                else{
                                        begin=temp_vmarea->vma_end;
                                }
                        }list_iterate_end();
                        if(ADDR_TO_PN(USER_MEM_HIGH)-begin>=npages)
                        {
                                return begin;
                        }
                }
                if(dir==VMMAP_DIR_HILO)
                {
                        vmarea_t* temp_vmarea=NULL;
                        uint32_t begin;
                        uint32_t end=ADDR_TO_PN(USER_MEM_HIGH);
                        list_iterate_reverse(&(map->vmm_list),temp_vmarea,vmarea_t,vma_plink)
                        {
                                begin=temp_vmarea->vma_end;
                                if(end-begin>=npages)
                                {
                                        return end-npages;
                                }
                                else{
                                        end=temp_vmarea->vma_start;
                                } 
                        }list_iterate_end();
                        if(end-ADDR_TO_PN(USER_MEM_LOW)>=npages)
                        {
                                return end-npages;
                        }
                }
        }
        return -1;
}

/* Find the vm_area that vfn lies in. Simply scan the address space
 * looking for a vma whose range covers vfn. If the page is unmapped,
 * return NULL. */
vmarea_t *
vmmap_lookup(vmmap_t *map, uint32_t vfn)
{
        /*NOT_YET_IMPLEMENTED("VM: vmmap_lookup");*/
        KASSERT(NULL != map);
        dbg(DBG_PRINT, "(GRADING3A 3.d)\n");

        if(list_empty(&(map->vmm_list))==1)
        {
                return NULL;
        }
        vmarea_t* temp_vmarea=NULL;
        list_iterate_begin(&(map->vmm_list),temp_vmarea,vmarea_t,vma_plink)
        {
                if(vfn>=temp_vmarea->vma_start&&vfn<temp_vmarea->vma_end)
                {
                        return temp_vmarea;
                }       
        }list_iterate_end();
        return NULL;
}

/* Allocates a new vmmap containing a new vmarea for each area in the
 * given map. The areas should have no mmobjs set yet. Returns pointer
 * to the new vmmap on success, NULL on failure. This function is
 * called when implementing fork(2). */
vmmap_t *
vmmap_clone(vmmap_t *map)
{
        /*NOT_YET_IMPLEMENTED("VM: vmmap_clone");*/
        vmmap_t *vmmap_copy=vmmap_create();
        if(vmmap_copy==NULL)
        {
                return NULL;
        }
        vmarea_t* temp_vmarea=NULL;
        list_iterate_begin(&(map->vmm_list),temp_vmarea,vmarea_t,vma_plink)
        {
                vmarea_t *new_vmarea_t=vmarea_alloc();
                if(new_vmarea_t==NULL)
                {
                        return NULL;
                }
                new_vmarea_t->vma_start=temp_vmarea->vma_start;
                new_vmarea_t->vma_end=temp_vmarea->vma_end;
                new_vmarea_t->vma_off=temp_vmarea->vma_off;
                new_vmarea_t->vma_prot=temp_vmarea->vma_prot;
                new_vmarea_t->vma_flags=temp_vmarea->vma_flags;

                new_vmarea_t->vma_obj=NULL;
                
                new_vmarea_t->vma_vmmap=vmmap_copy;

                list_link_init(&(new_vmarea_t->vma_plink));
                list_link_init(&(new_vmarea_t->vma_olink));

                vmmap_insert(vmmap_copy,new_vmarea_t);
        }list_iterate_end();
        return vmmap_copy;
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
        dbg(DBG_PRINT, "(GRADING3A 3.f)\n");
        KASSERT(0 < npages);
        dbg(DBG_PRINT, "(GRADING3A 3.f)\n");
        KASSERT(!(~(PROT_NONE | PROT_READ | PROT_WRITE | PROT_EXEC) & prot));
        dbg(DBG_PRINT, "(GRADING3A 3.f)\n");
        KASSERT((MAP_SHARED & flags) || (MAP_PRIVATE & flags));
        dbg(DBG_PRINT, "(GRADING3A 3.f)\n");
        KASSERT((0 == lopage) || (ADDR_TO_PN(USER_MEM_LOW) <= lopage));
        dbg(DBG_PRINT, "(GRADING3A 3.f)\n");
        KASSERT((0 == lopage) || (ADDR_TO_PN(USER_MEM_HIGH) >= (lopage + npages)));
        dbg(DBG_PRINT, "(GRADING3A 3.f)\n");
        KASSERT(PAGE_ALIGNED(off));
        dbg(DBG_PRINT, "(GRADING3A 3.f)\n");

	vmarea_t *new_vmarea_t=NULL;
	mmobj_t *newmmobj=NULL;

	if(lopage!=0)
        {  
                int res=vmmap_is_range_empty(map, lopage, npages);
	        if(!res)
                {
		        int res1=vmmap_remove(map, lopage, npages);
			if(res1!=0)
                        {
				return res1;
			}
		}
                new_vmarea_t=vmarea_alloc();
	        if(!new_vmarea_t)
                {
		        return -ENOMEM;
	        }
                new_vmarea_t->vma_start = lopage;
	        new_vmarea_t->vma_end = lopage+npages;
	        new_vmarea_t->vma_off = ADDR_TO_PN(off);
	        new_vmarea_t->vma_prot = prot;
	        new_vmarea_t->vma_flags = flags;
	}
        else
        {
	        int res=vmmap_find_range(map, npages, dir);
		if(res< 0)
                {
		        return -EINVAL;
		}
                new_vmarea_t= vmarea_alloc();
	        if(!new_vmarea_t)
                {
		        return -ENOMEM;
	        }
                new_vmarea_t->vma_start =res;
	        new_vmarea_t->vma_end =res+npages;
	        new_vmarea_t->vma_off = ADDR_TO_PN(off);
	        new_vmarea_t->vma_prot = prot;
	        new_vmarea_t->vma_flags = flags;
	}

	if(file==NULL)
        {
	        newmmobj=anon_create();
	        if(newmmobj==NULL)
                {
	                vmarea_free(new_vmarea_t);
		        return -ENOMEM;
                }

                if(new_vmarea_t->vma_flags&MAP_PRIVATE)
                {
                        mmobj_t* newshadowmmobj=shadow_create();
                        if(newshadowmmobj==NULL)
                        {
/*
                                list_remove(&new_vmarea_t->vma_olink);
                                newmmobj->mmo_ops->put(newmmobj);
*/
                                vmarea_free(new_vmarea_t);
		                return -ENOMEM;
                        }
                        newshadowmmobj->mmo_shadowed=newmmobj;
                        newshadowmmobj->mmo_un.mmo_bottom_obj=newmmobj;
                        new_vmarea_t->vma_obj=newshadowmmobj;        
                }
                else
                {
                        new_vmarea_t->vma_obj=newmmobj;
                }
/* new_vmarea_t->vma_obj=newmmobj;*/
                list_insert_tail(&newmmobj->mmo_un.mmo_vmas, &new_vmarea_t->vma_olink);
	        vmmap_insert(map,new_vmarea_t);
                if(new!=NULL)
                {
                        *new=new_vmarea_t;
                }
                return 0;
	}
        else
        {
	        int res=file->vn_ops->mmap(file,new_vmarea_t, &newmmobj);
		if(res) 
                {
		        vmarea_free(new_vmarea_t);
		        return res;
		}

                if(new_vmarea_t->vma_flags&MAP_PRIVATE)
                {
                        mmobj_t* newshadowmmobj=shadow_create();
                        if(newshadowmmobj==NULL)
                        {
/*
                                list_remove(&new_vmarea_t->vma_olink);
                                newmmobj->mmo_ops->put(newmmobj);
*/
                                vmarea_free(new_vmarea_t);
		                return -ENOMEM;
                        }
                        newshadowmmobj->mmo_shadowed=newmmobj;
                        newshadowmmobj->mmo_un.mmo_bottom_obj=newmmobj;
                        new_vmarea_t->vma_obj=newshadowmmobj;        
                }
                else
                {
                        new_vmarea_t->vma_obj=newmmobj;
                }
/* new_vmarea_t->vma_obj=newmmobj; */               
                list_insert_tail(&newmmobj->mmo_un.mmo_vmas, &new_vmarea_t->vma_olink);
	        vmmap_insert(map,new_vmarea_t);
                if(new!=NULL)
                {
                        *new=new_vmarea_t;
                }
                return 0;
	}
	return -1;
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
int
vmmap_remove(vmmap_t *map, uint32_t lopage, uint32_t npages)
{
        /*NOT_YET_IMPLEMENTED("VM: vmmap_remove");*/
        vmarea_t* temp_vmarea=NULL;
        uint32_t end=lopage+npages;
        list_iterate_begin(&(map->vmm_list),temp_vmarea,vmarea_t,vma_plink)
        {      
                if(temp_vmarea->vma_start>=end||lopage>=temp_vmarea->vma_end)
                {
        	          
                } 
                else if(lopage>temp_vmarea->vma_start&&end<temp_vmarea->vma_end)  
                {
                        vmarea_t *new_vmarea=vmarea_alloc();  
                        new_vmarea->vma_start = temp_vmarea->vma_start;
                        new_vmarea->vma_end = lopage;
                        new_vmarea->vma_off = temp_vmarea->vma_off;
                        new_vmarea->vma_prot = temp_vmarea->vma_prot;
                        new_vmarea->vma_flags =temp_vmarea->vma_flags;
                        new_vmarea->vma_vmmap = temp_vmarea->vma_vmmap;
                        new_vmarea->vma_obj = temp_vmarea->vma_obj;
                        new_vmarea->vma_obj->mmo_ops->ref(new_vmarea->vma_obj);
                        list_link_init(&new_vmarea->vma_plink);
                        list_link_init(&new_vmarea->vma_olink);
                        list_insert_before(&temp_vmarea->vma_plink, &new_vmarea->vma_plink);
                        /*list_insert_before(&temp_vmarea->vma_olink, &new_vmarea->vma_olink);*/

                        mmobj_t *bottom = mmobj_bottom_obj(temp_vmarea->vma_obj);        
                        if (bottom != temp_vmarea->vma_obj)
                        {               
                                list_insert_head(&bottom->mmo_un.mmo_vmas, &new_vmarea->vma_olink);
                        }
                        temp_vmarea->vma_off=end-temp_vmarea->vma_start+temp_vmarea->vma_off;
                        temp_vmarea->vma_start=end;       
                }    
                else if(lopage>temp_vmarea->vma_start&& end>=temp_vmarea->vma_end)  
                {
                        temp_vmarea->vma_end = lopage;      
                }
                else if(lopage<=temp_vmarea->vma_start &&end<temp_vmarea->vma_end)
                {
        	        temp_vmarea->vma_off=end-temp_vmarea->vma_start+temp_vmarea->vma_off;
                        temp_vmarea->vma_start=end;           
                }       
                else
                {
                        temp_vmarea->vma_obj->mmo_ops->put(temp_vmarea->vma_obj);
                        list_remove(&(temp_vmarea->vma_olink));
                        list_remove(&(temp_vmarea->vma_plink));
                        vmarea_free(temp_vmarea);
                }             
        }list_iterate_end();
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
        /*do we need to check the startvfn?????????????????????????????????????????*/
        uint32_t endvfn = startvfn+npages;
        KASSERT((startvfn < endvfn) && (ADDR_TO_PN(USER_MEM_LOW) <= startvfn) && (ADDR_TO_PN(USER_MEM_HIGH) >= endvfn));
        dbg(DBG_PRINT, "(GRADING3A 3.e)\n");
        
        if(list_empty(&(map->vmm_list))==1)
        {
                return 1;
        }
        vmarea_t* temp_vmarea=NULL;/****************how to avoid overlap when inserting into a sorted link******************/
        list_iterate_begin(&(map->vmm_list),temp_vmarea,vmarea_t,vma_plink)
        {
                if(((startvfn+npages)>temp_vmarea->vma_start)&&((startvfn+npages)<=temp_vmarea->vma_end))/**boundary right or not?**/
                {
                        return 0;
                }
                if((startvfn>=temp_vmarea->vma_start)&&(startvfn<temp_vmarea->vma_end))/**boundary right or not?**/
                {
                        return 0;
                }
                if((startvfn<=temp_vmarea->vma_start)&&((startvfn+npages)>=temp_vmarea->vma_end))
                {
                        return 0;
                }
        }list_iterate_end();
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
        /*NOT_YET_IMPLEMENTED("VM: vmmap_read");*/
        uint32_t res_vfn=ADDR_TO_PN(vaddr);
        vmarea_t *res_vmarea=vmmap_lookup(map,res_vfn);
        uint32_t pagenum1=res_vmarea->vma_off+res_vfn-res_vmarea->vma_start;
        pframe_t *res_pframe;
        int res1=pframe_lookup(res_vmarea->vma_obj,pagenum1,0,&res_pframe);
        if(res1<0)
        {
                return -EFAULT;
        }

        void *pf_startadd=(void *)((uintptr_t)(res_pframe->pf_addr)+PAGE_OFFSET(vaddr)); 
        
        size_t amount_read;
        if((PAGE_SIZE-PAGE_OFFSET(vaddr))>count)
        {
                amount_read=count;
        }
        else{
                amount_read=PAGE_SIZE-PAGE_OFFSET(vaddr);
        }
        memcpy(buf,pf_startadd,amount_read);
        count=count-amount_read;
        pagenum1++;
        buf=(void*)((uintptr_t)buf+amount_read);
        while(count>0)
        {
                size_t amount_read;
                if(PAGE_SIZE>count)
                {
                        amount_read=count;
                }
                else{
                        amount_read=PAGE_SIZE;
                }
                pframe_t *res_pframe;
                int res1=pframe_lookup(res_vmarea->vma_obj,pagenum1,0,&res_pframe);
                memcpy(buf,res_pframe->pf_addr,amount_read);
                pagenum1++;
                count=count-amount_read;
                buf=(void*)((uintptr_t)buf+amount_read);
        }       
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
        /*NOT_YET_IMPLEMENTED("VM: vmmap_write");*/
        uint32_t res_vfn=ADDR_TO_PN(vaddr);
        vmarea_t *res_vmarea=vmmap_lookup(map,res_vfn);
        uint32_t pagenum1=res_vmarea->vma_off+res_vfn-res_vmarea->vma_start;
        pframe_t *res_pframe;
        int res1=pframe_lookup(res_vmarea->vma_obj,pagenum1,0,&res_pframe);
        if(res1<0)
        {
                return -EFAULT;
        }

        void *pf_startadd=(void *)((uintptr_t)(res_pframe->pf_addr)+PAGE_OFFSET(vaddr)); 
        
        size_t amount_read;
        if((PAGE_SIZE-PAGE_OFFSET(vaddr))>count)
        {
                amount_read=count;
        }
        else{
                amount_read=PAGE_SIZE-PAGE_OFFSET(vaddr);
        }
        memcpy(pf_startadd,buf,amount_read);
        count=count-amount_read;
        pagenum1++;
        buf=(void*)((uintptr_t)buf+amount_read);
        while(count>0)
        {
                size_t amount_read;
                if(PAGE_SIZE>count)
                {
                        amount_read=count;
                }
                else{
                        amount_read=PAGE_SIZE;
                }
                pframe_t *res_pframe;
                int res1=pframe_lookup(res_vmarea->vma_obj,pagenum1,0,&res_pframe);
                memcpy(res_pframe->pf_addr,buf,amount_read);
                pagenum1++;
                count=count-amount_read;
                buf=(void*)((uintptr_t)buf+amount_read);
        }     
        return 0;
}
