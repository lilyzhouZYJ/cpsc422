#ifndef _KERN_FS_FLOCK_H_
#define _KERN_FS_FLOCK_H_

#ifdef _KERN_

#include <kern/lib/cv.h>

#define LOCK_SH 0x001  /* Shared lock */
#define LOCK_EX 0x002  /* Exclusive lock */
#define LOCK_UN 0x004  /* Remove existing lock */

#define LOCK_NB 0x008  /* Non-blocking request */

struct flock {
    enum { FLOCK_NONE, FLOCK_SH, FLOCK_EX } type;
    int num_shared_locks; // number of shared lock holders

    // Condition variables
    CV cv_shared_flock;    // waiting to acquire a shared lock
    CV cv_exclusive_flock; // waiting to acquire an exclusive lock// whether the struct file object is holding the flock
};

struct file;

void flock_init(struct flock * flock);

int flock_operation(struct file * f, int operation, int * errno);

// int flock_acquire(struct flock * flock, int type, int non_blocking, int * errno);
// int flock_release(struct flock * flock, int * errno);

#endif  /* _KERN_ */

#endif  /* !_KERN_FS_FLOCK_H_ */
