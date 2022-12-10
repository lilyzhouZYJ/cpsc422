#ifndef _KERN_LIB_KSTACK_H_
#define _KERN_LIB_KSTACK_H_

#ifdef _KERN_

#include <dev/intr.h>
#include <thread/PThread/export.h>

struct TQueue {
    unsigned int head;
    unsigned int tail;
};

struct CV {
    struct TQueue queue;
};

void CV_init(struct CV *cv);

void CV_wait(struct CV *cv, spinlock_t *lk);

void CV_signal(struct CV *cv);

#endif  /* _KERN_ */

#endif  /* !_KERN_LIB_KSTACK_H_ */
