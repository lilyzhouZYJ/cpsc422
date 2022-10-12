#include <lib/x86.h>
#include <lib/string.h>
#include <lib/debug.h>

#include "import.h"

#define PDE_ADDR(x) (x >> 22)
#define PTE_ADDR(x) ((x >> 12) & 0x3ff)

#define PAGESIZE      4096
#define PDIRSIZE      (PAGESIZE * 1024)
#define VM_USERLO     0x40000000
#define VM_USERHI     0xF0000000
#define VM_USERLO_PDE (VM_USERLO / PDIRSIZE)
#define VM_USERHI_PDE (VM_USERHI / PDIRSIZE)

/**
 * For each process from id 0 to NUM_IDS - 1,
 * set up the page directory entries so that the kernel portion of the map is
 * the identity map, and the rest of the page directories are unmapped.
 */
void pdir_init(unsigned int mbi_addr)
{
    unsigned int proc_index, pde_index;
    idptbl_init(mbi_addr);

    for (proc_index = 0; proc_index < NUM_IDS; proc_index++) {
        for (pde_index = 0; pde_index < 1024; pde_index++) {
            if ((pde_index < VM_USERLO_PDE) || (VM_USERHI_PDE <= pde_index)) {
                set_pdir_entry_identity(proc_index, pde_index);
            } else {
                rmv_pdir_entry(proc_index, pde_index);
            }
        }
    }
}

/**
 * Allocates a page (with container_alloc) for the page table,
 * and registers it in the page directory for the given virtual address,
 * and clears (set to 0) all page table entries for this newly mapped page table.
 * It returns the page index of the newly allocated physical page.
 * In the case when there's no physical page available, it returns 0.
 */
unsigned int alloc_ptbl(unsigned int proc_index, unsigned int vaddr)
{
    unsigned int page_index = container_alloc(proc_index);
    unsigned int pde_index = PDE_ADDR(vaddr);
    unsigned int pte_index;

    if (page_index == 0) {
        return 0;
    } else {
        set_pdir_entry_by_va(proc_index, vaddr, page_index);
        for (pte_index = 0; pte_index < 1024; pte_index++) {
            rmv_ptbl_entry(proc_index, pde_index, pte_index);
        }

        return page_index;
    }
}

// Reverse operation of alloc_ptbl.
// Removes corresponding the page directory entry,
// and frees the page for the page table entries (with container_free).
void free_ptbl(unsigned int proc_index, unsigned int vaddr)
{
    unsigned int page_index = get_pdir_entry_by_va(proc_index, vaddr) >> 12;

    rmv_pdir_entry(proc_index, PDE_ADDR(vaddr));
    container_free(proc_index, page_index);
}



// Copy the page table (2nd level) from one process to another.
// Also adjust the permissions correctly.
void copy_page_table(unsigned int from_proc_index, unsigned int to_proc_index)
{
	for(unsigned int pde_index = VM_USERLO_PDE; pde_index < VM_USERHI_PDE; pde_index++)
	{
		unsigned int pde = get_pdir_entry(from_proc_index, pde_index);
		if((pde & (PTE_U | PTE_P)) != 0){
			// Allocate 2nd-level page table
			unsigned int new_page_index = container_alloc(to_proc_index);
			if(new_page_index != 0){
				// Set the page directory point to the new page table
				set_pdir_entry(to_proc_index, pde_index, new_page_index);

				// Copy original page table to new page table by copying each PTE;
				// also modify the permissions for both
				for(unsigned int pte_index = 0; pte_index < 1024; pte_index++){
					unsigned int pte = get_ptbl_entry(from_proc_index, pde_index, pte_index);
					if((pte & (PTE_U | PTE_P)) != 0){
						unsigned int perm = PTE_P | PTE_U | PTE_COW; // write-only + COW
						set_ptbl_entry(to_proc_index, pde_index, pte_index, new_page_index, perm);

						// Modify permissions of original page table
						unsigned int orig_ptbl_entry = get_ptbl_entry(from_proc_index, pde_index, pte_index);
						unsigned int orig_page_index = orig_ptbl_entry >> 12;
						set_ptbl_entry(from_proc_index, pde_index, pte_index, orig_page_index, perm);
					}					
				}
			}
		}
	}
}

#define PT_PERM_PTU (PTE_P | PTE_W | PTE_U)

// Accomplish the "copy" part of copy-on-write
void copy_on_write(unsigned int pid, unsigned int vaddr)
{
	// Allocate new page for the vaddr
	unsigned int page_index = container_alloc(pid);
	if(page_index == 0){
		// TODO: what??
	} else {
		// Get original page
		unsigned int ptbl_entry = get_ptbl_entry_by_va(pid, vaddr);
		unsigned int orig_page_index = ptbl_entry >> 12;

		// Copy content of original page to new page
		void * orig_page = (void *) (orig_page_index << 12);
		void * new_page = (void *) (page_index << 12);
		memcpy(new_page, orig_page, PAGESIZE);

		set_ptbl_entry_by_va(pid, vaddr, page_index, PT_PERM_PTU);
	}
}