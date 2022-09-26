#include <lib/x86.h>

#include "import.h"

/**
 * Returns the page table entry corresponding to the virtual address,
 * according to the page structure of process # [proc_index].
 * Returns 0 if the mapping does not exist.
 */
unsigned int get_ptbl_entry_by_va(unsigned int proc_index, unsigned int vaddr)
{
    // TODO
    unsigned int pde_index = vaddr >> 22;
    unsigned int pte_index = (vaddr >> 12) | 0x3ff;
    unsigned int ptbl_entry = get_ptbl_entry(proc_index, pde_index, pte_index);

    // Check if the mapping exists
    if((ptbl_entry & 0x1) == 0){
        return 0;
    } else {
        return ptbl_entry;
    }
}

// Returns the page directory entry corresponding to the given virtual address.
unsigned int get_pdir_entry_by_va(unsigned int proc_index, unsigned int vaddr)
{
    // TODO
    unsigned int pde_index = vaddr >> 22;
    return get_pdir_entry(proc_index, pde_index);
}

// Removes the page table entry for the given virtual address.
void rmv_ptbl_entry_by_va(unsigned int proc_index, unsigned int vaddr)
{
    // TODO
    unsigned int pde_index = vaddr >> 22;
    unsigned int pte_index = (vaddr >> 12) | 0x3ff;
    rmv_ptbl_entry(proc_index, pde_index, pte_index);
}

// Removes the page directory entry for the given virtual address.
void rmv_pdir_entry_by_va(unsigned int proc_index, unsigned int vaddr)
{
    // TODO
    unsigned int pde_index = vaddr >> 22;
    rmv_pdir_entry(proc_index, pde_index);
}

// Maps the virtual address [vaddr] to the physical page # [page_index] with permission [perm].
// You do not need to worry about the page directory entry. just map the page table entry.
void set_ptbl_entry_by_va(unsigned int proc_index, unsigned int vaddr,
                          unsigned int page_index, unsigned int perm)
{
    // TODO
    unsigned int pde_index = vaddr >> 22;
    unsigned int pte_index = (vaddr >> 12) | 0x3ff;
    set_ptbl_entry(proc_index, pde_index, pte_index, page_index, perm);
}

// Registers the mapping from [vaddr] to physical page # [page_index] in the page directory.
void set_pdir_entry_by_va(unsigned int proc_index, unsigned int vaddr,
                          unsigned int page_index)
{
    // TODO
    unsigned int pde_index = vaddr >> 22;
    set_pdir_entry(proc_index, pde_index, page_index);
}

// Initializes the identity page table.
// The permission for the kernel memory should be PTE_P, PTE_W, and PTE_G,
// While the permission for the rest should be PTE_P and PTE_W.
void idptbl_init(unsigned int mbi_addr)
{
    // TODO: Define your local variables here.

    container_init(mbi_addr);

    // TODO
    // for(unsigned int page_id = 0; page_id < NUM_PAGES; page_id++){
    //     unsigned int pde_index = page_id >> 10;
    //     unsigned int pte_index = page_id & 0x3ff;
    //     if(page_id >= VM_USERLO_PI && page_id < VM_USERHI_PI){
    //         // user page
    //         set_ptbl_entry_identity(pde_index, pte_index, PTE_P | PTE_W);
    //     } else {
    //         // kernel page
    //         set_ptbl_entry_identity(pde_index, pte_index, PTE_P | PTE_W | PTE_G);
    //     }
    // }
}
