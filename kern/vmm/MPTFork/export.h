#ifndef _KERN_VMM_MPTCOMM_H_
#define _KERN_VMM_MPTCOMM_H_

#ifdef _KERN_

void copy_page_table(unsigned int from_proc_index, unsigned int to_proc_index);
void copy_on_write(unsigned int pid, unsigned int vaddr);

#endif  /* _KERN_ */

#endif  /* !_KERN_VMM_MPTCOMM_H_ */
