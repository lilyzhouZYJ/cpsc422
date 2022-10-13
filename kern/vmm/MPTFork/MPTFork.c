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

// Copy the page table from one process to another,
// including both pdir and 2nd-level page table.
// Also adjust the permissions accordingly.
// TODO: handle error!
void copy_page_table(unsigned int from_proc_index, unsigned int to_proc_index)
{
	for(unsigned int pde_index = VM_USERLO_PDE; pde_index < VM_USERHI_PDE; pde_index++)
	{
		// Get PDE of old process
		unsigned int pde = get_pdir_entry(from_proc_index, pde_index);
		if((pde & PTE_P) != PTE_P){
			// PDE not present
			continue;
		}

		// Allocate 2nd-level page table for new process
		unsigned int vaddr = pde_index << 22; // dummy vaddr for alloc_ptbl
		unsigned int page_table_index = alloc_ptbl(to_proc_index, vaddr);
		if(page_table_index == 0){
			// Failed to allocate physical page
			continue;
		}

		// Copy page table of old process to new page table by copying each PTE.
		// Also modify the permissions for both.
		for(unsigned int pte_index = 0; pte_index < 1024; pte_index++)
		{
			// Get the PTE of the old process
			unsigned int pte = get_ptbl_entry(from_proc_index, pde_index, pte_index);
			if((pte & PTE_P) != PTE_P){
				// PTE not present
				continue;
			}

			// Page index that this PTE is pointing to
			unsigned int page_index = pte >> 12;

			// Set up new permissions
			if((pte & PTE_W) == PTE_W || (pte & PTE_COW) == PTE_COW){
				// Original PTE is writeable => set copy-on-write bit for both processes
				unsigned int perm = (PTE_COW | PTE_P | PTE_U);
				set_ptbl_entry(to_proc_index, pde_index, pte_index, page_index, perm);
				set_ptbl_entry(from_proc_index, pde_index, pte_index, page_index, perm);
			} else {
				// Original PTE is not writeable => make no change to permission
				set_ptbl_entry(to_proc_index, pde_index, pte_index, page_index, PTE_P | PTE_U);
			}
		}
	}
}

// Accomplish the "copy" part of copy-on-write
void copy_on_write(unsigned int pid, unsigned int vaddr)
{
	// Get original page
	unsigned int orig_page_index = get_ptbl_entry_by_va(pid, vaddr) >> 12;

	// Allocate new page for the vaddr
	unsigned int pdir_page_index = alloc_page(pid, vaddr, PT_PERM_PTU);

	if(pdir_page_index == MagicNumber){
		// Failed to allocate page
		return;
	} else {
		// Get new page
		unsigned int new_page_index = get_ptbl_entry_by_va(pid, vaddr) >> 12;

		// Copy content of original page to new page
		void * orig_page = (void *) (orig_page_index << 12);
		void * new_page = (void *) (new_page_index << 12);
		memcpy(new_page, orig_page, PAGESIZE);
	}
}