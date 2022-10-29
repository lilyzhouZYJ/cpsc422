#ifndef _KERN_THREAD_CVTHREADQUEUE_H_
#define _KERN_THREAD_CVTHREADQUEUE_H_

#ifdef _KERN_

struct TQueue;

void tcb_set_prev(unsigned int pid, unsigned int prev_pid);
void tcb_set_next(unsigned int pid, unsigned int next_pid);
unsigned int tcb_get_next(unsigned int pid);

#endif  /* _KERN_ */

#endif  /* !_KERN_THREAD_CVTHREADQUEUE_H_ */
