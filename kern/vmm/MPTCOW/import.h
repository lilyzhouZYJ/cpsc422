#ifndef _KERN_VMM_MPTCOW_H_
#define _KERN_VMM_MPTCOW_H_

#ifdef _KERN_

void copy_pdir_entry(unsigned int from, unsigned int to, unsigned int pde);
unsigned int rmv_ptbl_entry_by_va(unsigned int proc_index, unsigned int vaddr);
unsigned int alloc_page(unsigned int proc_index, unsigned int vaddr, unsigned int perm);
unsigned int get_ptbl_entry_by_va(unsigned int proc_index, unsigned int vaddr);

#endif  /* _KERN_ */

#endif  /* !_KERN_VMM_MPTCOW_H_ */
