#include <lib/x86.h>
#include <lib/debug.h>

#include "import.h"

// Remove permissions from page table entry / page directory entry
unsigned int removePerms(unsigned int entry);

/**
 * Sets the entire page map for process 0 as the identity map.
 * Note that part of the task is already completed by pdir_init.
 */
void pdir_init_kern(unsigned int mbi_addr)
{
    // TODO: Define your local variables here.

    pdir_init(mbi_addr);

    //TODO
    for(unsigned int pde_index = 0; pde_index < 1024; pde_index++){
        set_pdir_entry_identity(0, pde_index);
    }
}

/**
 * Maps the physical page # [page_index] for the given virtual address with the given permission.
 * In the case when the page table for the page directory entry is not set up,
 * you need to allocate the page table first.
 * In the case of error, it returns the constant MagicNumber defined in lib/x86.h,
 * otherwise, it returns the physical page index registered in the page directory,
 * (the return value of get_pdir_entry_by_va or alloc_ptbl).
 */
unsigned int map_page(unsigned int proc_index, unsigned int vaddr,
                      unsigned int page_index, unsigned int perm)
{
    // TODO
    unsigned int page_dir_entry = get_pdir_entry_by_va(proc_index, vaddr);
    if((page_dir_entry & PTE_P) == 0){
        // Page is not set up; need to allocate the page table
        unsigned int page_table_index = alloc_ptbl(proc_index, vaddr);
        if(page_table_index == 0){
            // No page can be allocated
            return MagicNumber;
        }
    }

    set_ptbl_entry_by_va(proc_index, vaddr, page_index, perm);

    return get_pdir_entry_by_va(proc_index, vaddr);
}

/**
 * Remove the mapping for the given virtual address (with rmv_ptbl_entry_by_va).
 * You need to first make sure that the mapping is still valid,
 * e.g., by reading the page table entry for the virtual address.
 * Nothing should be done if the mapping no longer exists.
 * You do not need to unmap the page table from the page directory.
 * It should return the corresponding page table entry.
 */
unsigned int unmap_page(unsigned int proc_index, unsigned int vaddr)
{
    // TODO
    unsigned int page_dir_entry = get_pdir_entry_by_va(proc_index, vaddr);
    if((page_dir_entry & PTE_P) == 0){
        // Second-level page table is not present
        return 0;
    }

    unsigned int page_table_entry = get_ptbl_entry_by_va(proc_index, vaddr);
    if((page_table_entry & PTE_P) == 0){
        // Page is not present
        return 0;
    }

    rmv_ptbl_entry_by_va(proc_index, vaddr);
    return page_table_entry;
}
