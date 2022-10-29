#ifndef _KERN_THREAD_BBQUEUE_H_
#define _KERN_THREAD_BBQUEUE_H_

#ifdef _KERN_

void BBQ_init();
void BBQ_insert(int n);
int BBQ_remove();

#endif  /* _KERN_ */

#endif  /* !_KERN_THREAD_BBQUEUE_H_ */
