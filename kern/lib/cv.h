#ifndef _KERN_LIB_CV_H_
#define _KERN_LIB_CV_H_

#ifdef _KERN_

#include <lib/spinlock.h>
#include <lib/x86.h>

typedef struct {
    unsigned int tail;
    unsigned int queue[NUM_IDS];
} CV;

typedef struct {
#define BUFFER_SIZE 3
    unsigned int buf[BUFFER_SIZE];
    unsigned int head;
    unsigned int size;
    spinlock_t lk;
    CV empty;
    CV full;
} BoundedBuffer;

void CV_init(CV *cv);
void CV_wait(CV *cv, spinlock_t *lk);
void CV_signal(CV *cv);
void CV_broadcast(CV *cv);

void BB_init(BoundedBuffer *bb);
bool BB_is_empty(BoundedBuffer *bb);
bool BB_is_full(BoundedBuffer *bb);
void BB_enqueue(BoundedBuffer *bb, unsigned int val);
unsigned int BB_dequeue(BoundedBuffer *bb);

#endif  /* _KERN_ */

#endif  /* !_KERN_LIB_CV_H_ */
