#ifndef _KERN_THREAD_CONDVARIABLE_H_
#define _KERN_THREAD_CONDVARIABLE_H_

#ifdef _KERN_

#include <lib/spinlock.h>

struct CV;

void CV_init(struct CV *cv);
void CV_wait(struct CV *cv, spinlock_t *lk);
void CV_signal(struct CV *cv);

#endif  /* _KERN_ */

#endif  /* !_KERN_THREAD_CONDVARIABLE_H_ */
