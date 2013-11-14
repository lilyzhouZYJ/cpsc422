#ifndef _KERN_VIRT_SVM_INTRO_H_
#define _KERN_VIRT_SVM_INTRO_H_

#ifdef _KERN_

unsigned int xvmst_read(unsigned int ofs);
void xvmst_write(unsigned int ofs, unsigned int v);

/*
 * Primitives derived from lower layers.
 */

unsigned int palloc(void);
void pfree(unsigned int idx);

unsigned int pt_read(unsigned int pid, unsigned int va);
void pt_resv(unsigned int pid, unsigned int vaddr, unsigned int perm);

unsigned int pt_copyin(unsigned int pmap_id,
		       unsigned int uva, char *kva, unsigned int len);
unsigned int pt_copyout(char *kva,
			unsigned int pmap_id, unsigned int uva, unsigned int len);
unsigned int pt_memset(unsigned int pmap_id,
		       unsigned int va, char c, unsigned int len);

unsigned int get_curid(void);

void thread_kill(unsigned int pid, unsigned chid);

void thread_wakeup(unsigned int chid);
void thread_sleep(void);
void thread_yield(void);

void uctx_set(unsigned int pid, unsigned int idx, unsigned int val);
unsigned int uctx_get(unsigned int pid, unsigned int idx);

void npt_insert(unsigned int gpa, unsigned int hpa);

void switch_to_guest(void);
void switch_to_host(void);

unsigned int vmcb_read_z(unsigned int ofs);
unsigned int vmcb_read_v(unsigned int ofs);

void vmcb_write_z(unsigned int ofs, unsigned int v);
void vmcb_write_v(unsigned int ofs, unsigned int v);

void vmcb_init(unsigned int mbi_addr);

#endif /* _KERN_ */

#endif /* !_KERN_VIRT_SVM_INTRO_H_ */
