#ifndef _KERN_THREAD_PKCTXNEW_H_
#define _KERN_THREAD_PKCTXNEW_H_

#ifdef _KERN_

// Determines whether the process # [id] can consume an extra
// [n] pages of memory. If so, returns 1, otherwise, returns 0.
unsigned int container_can_consume(unsigned int id, unsigned int n);

// Designate some memory quota for the next child process.
unsigned int alloc_mem_quota(unsigned int id, unsigned int quota);

void kctx_set_esp(unsigned int pid, void *esp);
void kctx_set_eip(unsigned int pid, void *eip);

#endif  /* _KERN_ */

#endif  /* !_KERN_THREAD_PKCTXNEW_H_ */
