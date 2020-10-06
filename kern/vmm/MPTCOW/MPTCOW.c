#include <lib/string.h>
#include <lib/x86.h>

#include "import.h"

#define PAGESIZE      4096
#define PDIRSIZE      (PAGESIZE * 1024)
#define VM_USERLO     0x40000000
#define VM_USERHI     0xF0000000
#define VM_USERLO_PDE (VM_USERLO / PDIRSIZE)
#define VM_USERHI_PDE (VM_USERHI / PDIRSIZE)

#define PT_PERM_PTU (PTE_P | PTE_W | PTE_U)

#define ADDR_MASK(x) ((unsigned int) x & 0xfffff000)

/* Copy memory map from 'from' to 'to' and mark the copied entries as COW.
 * N.B. Only user space mapping has to be copied. */
void map_cow(unsigned int from, unsigned int to)
{
    unsigned int pde;
    for (pde = VM_USERLO_PDE; pde < VM_USERHI_PDE; pde++) {
        copy_pdir_entry(from, to, pde);
    }
}

/* Allocate a new page frame, and copy contents from the previous one. */
void map_decow(unsigned int pid, unsigned int vaddr)
{
    unsigned int pde_index, pte_index, pte_entry;
    unsigned int *from_pa, *to_pa;

    from_pa = (unsigned int *) ADDR_MASK(get_ptbl_entry_by_va(pid, vaddr));
    rmv_ptbl_entry_by_va(pid, vaddr);

    alloc_page(pid, vaddr, PT_PERM_PTU);
    to_pa = (unsigned int *) ADDR_MASK(get_ptbl_entry_by_va(pid, vaddr));

    memcpy(to_pa, from_pa, 4096);
}
