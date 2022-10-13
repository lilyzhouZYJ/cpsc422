#include <lib/x86.h>
#include <lib/string.h>
#include <lib/debug.h>

#include "import.h"

#define PAGESIZE      4096
#define PDIRSIZE      (PAGESIZE * 1024)
#define VM_USERLO     0x40000000
#define VM_USERHI     0xF0000000
#define VM_USERLO_PDE (VM_USERLO / PDIRSIZE)
#define VM_USERHI_PDE (VM_USERHI / PDIRSIZE)

#define PT_PERM_PTU (PTE_P | PTE_W | PTE_U)

// Copy the page table from one process to another.
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
					if(pte & (PTE_P | PTE_U)){
						unsigned int perm = PTE_P | PTE_U;
						if(pte & PTE_W){
							// Original PTE is writeable => set copy-on-write bit
							perm = perm | PTE_COW;
						}
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

// Accomplish the "copy" part of copy-on-write
void copy_on_write(unsigned int pid, unsigned int vaddr)
{
	// Allocate new page for the vaddr
	unsigned int page_index = alloc_page(pid, vaddr, PT_PERM_PTU);
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
	}
}