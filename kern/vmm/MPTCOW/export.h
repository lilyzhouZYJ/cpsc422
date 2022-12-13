#ifndef _KERN_VMM_MPTCOW_H_
#define _KERN_VMM_MPTCOW_H_

#ifdef _KERN_

void map_cow(unsigned int from, unsigned int to);
void map_decow(unsigned int pid, unsigned int vaddr);
unsigned int container_split(unsigned int id, unsigned int quota);
unsigned int alloc_page(unsigned int pid, unsigned int vaddr, unsigned int perm);
void set_pdir_base(unsigned int index);

#endif  /* _KERN_ */

#endif  /* !_KERN_VMM_MPTCOW_H_ */
