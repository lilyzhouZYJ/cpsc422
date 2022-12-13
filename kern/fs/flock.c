#include <kern/lib/spinlock.h>
#include <kern/lib/debug.h>
#include <kern/lib/syscall.h>
#include <thread/PCurID/export.h>
#include "flock.h"
#include "file.h"

// Spinlock
spinlock_t flock_lk;

void flock_init(struct flock * flock)
{
    spinlock_init(&flock_lk);

    // Initialize flock attributes
    flock->type = FLOCK_NONE;
    flock->num_shared_locks = 0;

    CV_init(&(flock->cv_shared_flock));
    CV_init(&(flock->cv_exclusive_flock));
}

int flock_acquire(struct file * f, int type, int non_blocking, int * errno)
{
    // Acquire spinlock
    spinlock_acquire(&flock_lk);

    // Get pid of current process
    int curid = get_curid();

    struct flock * flock = &(f->ip->flock);
    KERN_ASSERT(f->hold_flock == 0);

    // KERN_DEBUG("=== flock_acquire: curid %d, type %d, non_blocking %d ===\n", curid, type, non_blocking);

    // Sanity check
    if(type != FLOCK_SH && type != FLOCK_EX){
        KERN_PANIC("flock_acquire: lock type %d is invalid\n", type);
    }

    // (1) Acquire shared lock
    if(type == FLOCK_SH){
        // (a) If current lock is exclusive
        if(flock->type == FLOCK_EX){
            if(non_blocking == 1){
                // Reject the request
                KERN_DEBUG("flock_acquire: curid %d tried to acquire shared lock, failed, will quit\n", curid);
                *errno = E_WOULDBLOCK;
                spinlock_release(&flock_lk);
                return -1;
            } else {
                // Put the process on waiting list
                while(flock->type == FLOCK_EX){
                    KERN_DEBUG("flock_acquire: curid %d tried to acquire shared lock, failed, will be suspended\n", curid);
                    CV_wait(&flock->cv_shared_flock, &flock_lk);
                }
                
                KERN_DEBUG("flock_acquire: curid %d is woken up and acquires shared lock\n", curid);
                KERN_ASSERT(flock->type != FLOCK_EX);

                // Acquire the lock
                flock->type = FLOCK_SH;
                flock->num_shared_locks++;
                f->hold_flock = 1;
                spinlock_release(&flock_lk);
                return 0;
            }
        }
        // (b) If current lock is shared / none
        else {
            // Acquire the lock
            KERN_DEBUG("flock_acquire: curid %d acquires shared lock\n", curid);
            flock->type = FLOCK_SH;
            flock->num_shared_locks++;
            f->hold_flock = 1;
            spinlock_release(&flock_lk);
            return 0;
        }
    } 
    // (2) Acquire exclusive lock
    else if (type == FLOCK_EX){
        // (a) If current lock is none
        if(flock->type == FLOCK_NONE){
            KERN_ASSERT(flock->num_shared_locks == 0);

            // Acquire the lock
            KERN_DEBUG("flock_acquire: curid %d acquires exclusive lock\n", curid);
            flock->type = FLOCK_EX;
            f->hold_flock = 1;
            spinlock_release(&flock_lk);
            return 0;
        }
        // (b) If current lock is shared / exclusive
        else {
            if(non_blocking == 1){
                // Reject the request
                KERN_DEBUG("flock_acquire: curid %d tried to acquire exclusive lock, failed, will quit\n", curid);
                *errno = E_WOULDBLOCK;
                spinlock_release(&flock_lk);
                return -1;
            } else {
                // Put the process on waiting list
                while(flock->type != FLOCK_NONE){
                    KERN_DEBUG("flock_acquire: curid %d tried to acquire exclusive lock, failed, will be suspended\n", curid);
                    CV_wait(&flock->cv_exclusive_flock, &flock_lk);
                }

                KERN_ASSERT(flock->type == FLOCK_NONE && flock->num_shared_locks == 0);
                KERN_DEBUG("flock_acquire: curid %d is woken up and acquires exclusive lock\n", curid);

                // Acquire the lock
                flock->type = FLOCK_EX;
                f->hold_flock = 1;
                spinlock_release(&flock_lk);
                return 0;
            }
        }
    }

    KERN_PANIC("flock_acquire: should not reach here\n");
    return -1;
}

int flock_release(struct file * f, int * errno)
{
    // Acquire spinlock
    spinlock_acquire(&flock_lk);

    // Get pid of current process
    int curid = get_curid();
    // KERN_DEBUG("=== flock_release: curid %d ===\n", curid);

    struct flock * flock = &(f->ip->flock);

    if(flock->type == FLOCK_NONE || f->hold_flock == 0){
        // File is not locked, or current file object does not hold a lock
        *errno = E_INVAL;
        KERN_DEBUG("flock_release: error, flock->type is %d, f->hold_flock is %d\n", flock->type, f->hold_flock);
        spinlock_release(&flock_lk);
        return -1;
    }

    // (1) Release shared lock
    if(flock->type == FLOCK_SH){
        KERN_ASSERT(flock->num_shared_locks > 0);
        KERN_DEBUG("flock_release: curid %d releases shared lock\n", curid);

        flock->num_shared_locks--;

        if(flock->num_shared_locks == 0){
            KERN_DEBUG("flock_release: all shared lock released\n");
            flock->type = FLOCK_NONE;
        }
    }
    // (2) Release exclusive lock
    else {
        KERN_DEBUG("flock_release: curid %d is releasing exclusive lock\n", curid);
        flock->type = FLOCK_NONE;
    }

    KERN_ASSERT(flock->type != FLOCK_EX);
    
    // Set current struct file object to be no longer holding a flock
    f->hold_flock = 0;

    // Determine what waiting requests could be granted now
    if(flock->type == FLOCK_NONE){
        // (1)Any waiting requests could be satisfied
        KERN_DEBUG("flock_release: any request could be granted due to release\n");
        CV_signal(&flock->cv_shared_flock);
        CV_signal(&flock->cv_exclusive_flock);
    } else if (flock->type == FLOCK_SH){
        // (2) Only waiting requests for shared locks could be satisfied
        KERN_DEBUG("flock_release: shared-flock request could be granted due to release\n");
        CV_signal(&flock->cv_shared_flock);
    }

    // Release spinlock
    spinlock_release(&flock_lk);
    return 0;
}

int flock_operation(struct file * f, int operation, int * errno)
{
    int non_blocking = (operation & LOCK_NB) > 0 ? 1 : 0;
    
    // int curid = get_curid();
    // KERN_DEBUG("=== flock_operation: curid %d, operation %d ===\n", curid, operation);

    if((operation & LOCK_SH) == LOCK_SH){
        // Acquire shared lock
        if((operation & (LOCK_EX | LOCK_UN)) > 0){
            // Invalid operation
            *errno = E_INVAL;
            return -1;
        }

        if(f->hold_flock == 1){
            // The file struct object is holding the flock;
            // need to release first
            int ret = flock_release(f, errno);
            if(ret < 0){
                return -1;
            }
        }
        return flock_acquire(f, FLOCK_SH, non_blocking, errno);
    }
    else if ((operation & LOCK_EX) == LOCK_EX){
        // Acquire exclusive lock
        if((operation & (LOCK_SH | LOCK_UN)) > 0){
            // Invalid operation
            *errno = E_INVAL;
            return -1;
        }

        if(f->hold_flock == 1){
            // The file struct object is holding the flock;
            // need to release first
            int ret = flock_release(f, errno);
            if(ret < 0){
                return -1;
            }
        }
        return flock_acquire(f, FLOCK_EX, non_blocking, errno);
    }
    else if ((operation & LOCK_UN) == LOCK_UN){
        // Release lock
        if((operation & (LOCK_SH | LOCK_EX)) > 0){
            // Invalid operation
            *errno = E_INVAL;
            return -1;
        }

        return flock_release(f, errno);
    }
    else {
        // Invalid operation
        *errno = E_INVAL;
        return -1;
    }
}