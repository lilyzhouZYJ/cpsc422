#include <lib/gcc.h>
#include <lib/x86.h>
#include <lib/debug.h>

#include "import.h"

#define PT_PERM_UP  0
#define PT_PERM_PTU (PTE_P | PTE_W | PTE_U)

/**
 * Page directory pool for NUM_IDS processes.
 * mCertiKOS maintains one page structure for each process.
 * Each PDirPool[index] represents the page directory of the page structure
 * for the process # [index].
 * In mCertiKOS, we statically allocate page directories, and maintain the second
 * level page tables dynamically.
 * The unsigned int * type is meant to suggest that the contents of the array
 * are pointers to page tables. In reality they are actually page directory
 * entries, which are essentially pointers plus permission bits. The functions
 * in this layer will require casting between integers and pointers anyway and
 * in fact any 32-bit type is fine, so feel free to change it if it makes more
 * sense to you with a different type.
 */
unsigned int *PDirPool[NUM_IDS][1024] gcc_aligned(PAGESIZE);

/**
 * In mCertiKOS, we use identity page table mappings for the kernel memory.
 * IDPTbl is an array of statically allocated identity page tables that will be
 * reused for all the kernel memory.
 * That is, in every page directory, the entries that fall into the range of
 * addresses reserved for the kernel will point to an entry in IDPTbl.
 */
unsigned int IDPTbl[1024][1024] gcc_aligned(PAGESIZE);

// Remove permissions from page table entry / page directory entry
unsigned int removePerms(unsigned int entry){
    entry = ((entry >> 12) << 12);
    return entry;
}

// Sets the CR3 register with the start address of the page structure for process # [index].
void set_pdir_base(unsigned int index)
{
    // TODO
	set_cr3(PDirPool[index]);
}

// Returns the page directory entry # [pde_index] of the process # [proc_index].
// This can be used to test whether the page directory entry is mapped.
unsigned int get_pdir_entry(unsigned int proc_index, unsigned int pde_index)
{
    // TODO
    return (unsigned int) PDirPool[proc_index][pde_index];
}

// Sets the specified page directory entry with the start address of physical
// page # [page_index].
// You should also set the permissions PTE_P, PTE_W, and PTE_U.
void set_pdir_entry(unsigned int proc_index, unsigned int pde_index,
                    unsigned int page_index)
{
    // TODO
	uintptr_t page_dir_entry = page_index << 12;
	page_dir_entry = (page_dir_entry | PT_PERM_PTU);

	PDirPool[proc_index][pde_index] = (unsigned int *) page_dir_entry;
}

// Sets the page directory entry # [pde_index] for the process # [proc_index]
// with the initial address of page directory # [pde_index] in IDPTbl.
// You should also set the permissions PTE_P, PTE_W, and PTE_U.
// This will be used to map a page directory entry to an identity page table.
void set_pdir_entry_identity(unsigned int proc_index, unsigned int pde_index)
{
    // TODO
	uintptr_t addr = (uintptr_t) IDPTbl[pde_index]; // address of page directory # [pde_index] in IDPTbl
	PDirPool[proc_index][pde_index] = (unsigned int *) (addr | PT_PERM_PTU);
}

// Removes the specified page directory entry (sets the page directory entry to 0).
// Don't forget to cast the value to (unsigned int *).
void rmv_pdir_entry(unsigned int proc_index, unsigned int pde_index)
{
    // TODO
    PDirPool[proc_index][pde_index] = (unsigned int *) 0;
}

// Returns the specified page table entry.
// Do not forget that the permission info is also stored in the page directory entries.
unsigned int get_ptbl_entry(unsigned int proc_index, unsigned int pde_index,
                            unsigned int pte_index)
{
    // TODO
    unsigned int pdir_entry = get_pdir_entry(proc_index, pde_index);

    // Check if the page directory entry is mapped
    if((pdir_entry & PTE_P) > 0){
        // Remove permissions
        pdir_entry = removePerms(pdir_entry);
        return ((unsigned int *)pdir_entry)[pte_index];
    } else {
        return 0;
    }
}

// Sets the specified page table entry with the start address of physical page # [page_index]
// You should also set the given permission.
void set_ptbl_entry(unsigned int proc_index, unsigned int pde_index,
                    unsigned int pte_index, unsigned int page_index,
                    unsigned int perm)
{
    // TODO
    unsigned int pdir_entry = get_pdir_entry(proc_index, pde_index);
    // check if the page table is present
    if((pdir_entry & PTE_P) > 0){
        // Remove permissions
        pdir_entry = removePerms(pdir_entry);
        // go into the page table
        unsigned int * ptable = (unsigned int *) pdir_entry;
        ptable[pte_index] = ((page_index << 12) | perm);
    }
}

// Sets up the specified page table entry in IDPTbl as the identity map.
// You should also set the given permission.
void set_ptbl_entry_identity(unsigned int pde_index, unsigned int pte_index,
                             unsigned int perm)
{
    // TODO
    IDPTbl[pde_index][pte_index] = (((pde_index << 22) + (pte_index << 12)) | perm);
}

// Sets the specified page table entry to 0.
void rmv_ptbl_entry(unsigned int proc_index, unsigned int pde_index,
                    unsigned int pte_index)
{
    // TODO
    unsigned int pdir_entry = get_pdir_entry(proc_index, pde_index);
    // check if the page table is present
    if((pdir_entry & PTE_P) > 0){
        // Remove permissions
        pdir_entry = removePerms(pdir_entry);
        // go into the page table
        unsigned int * ptable = (unsigned int *) pdir_entry;
        ptable[pte_index] = 0;
    }
}
