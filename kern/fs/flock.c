#include <kern/lib/spinlock.h>
#include <thread/PCurID/export.h>
#include "flock.h"

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

    for(int i = 0; i < NUM_IDS; i++){
        lock_holder[i] = 0;
    }
}

int flock_acquire(struct flock * flock, int type, int non_blocking, int * errno)
{
    // Acquire spinlock
    spinlock_acquire(&flock_lk);

    // Get pid of current process
    int curid = get_curid();
    KERN_ASSERT(flock->lock_holder[curid] == 0);

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
                *errno = EWOULDBLOCK;
                spinlock_release(&flock_lk);
                return -1;
            } else {
                // Put the process on waiting list
                while(flock->type == FLOCK_EX){
                    CV_wait(&flock->cv_shared_flock, &flock_lk);
                }

                KERN_ASSERT(flock->type != FLOCK_EX);

                // Acquire the lock
                flock->type = FLOCK_SH;
                flock->num_shared_locks++;
                flock->lock_holder[curid] = 1;
                spinlock_release(&flock_lk);
                return 0;
            }
        }
        // (b) If current lock is shared / none
        else {
            // Acquire the lock
            flock->type = FLOCK_SH;
            flock->num_shared_locks++;
            flock->lock_holder[curid] = 1;
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
            flock->type = FLOCK_EX;
            flock->lock_holder[curid] = 1;
            spinlock_release(&flock_lk);
            return 0;
        }
        // (b) If current lock is shared / exclusive
        else {
            if(non_blocking == 1){
                // Reject the request
                *errno = EWOULDBLOCK;
                spinlock_release(&flock_lk);
                return -1;
            } else {
                // Put the process on waiting list
                while(flock->type != FLOCK_NONE){
                    CV_wait(&flock->cv_exclusive_flock, &flock_lk);
                }

                KERN_ASSERT(flock->type == FLOCK_NONE && flock->num_shared_locks == 0);

                // Acquire the lock
                flock->type = FLOCK_EX;
                flock->lock_holder[curid] = 1;
                spinlock_release(&flock_lk);
                return 0;
            }
        }
    }
}

int flock_release(struct flock * flock, int * errno)
{
    // Acquire spinlock
    spinlock_acquire(&flock_lk);

    // Get pid of current process
    int curid = get_curid();

    if(flock->type == FLOCK_NONE || flock->lock_holder[curid] == 0){
        // File is not locked, or current process does not hold a lock
        *errno = EINVAL;
        spinlock_release(&flock_lk);
        return -1;
    }

    // (1) Release shared lock
    if(flock->type == FLOCK_SH){
        KERN_ASSERT(flock->num_shared_locks > 0);

        flock->num_shared_locks--;

        if(flock->num_shared_locks == 0){
            flock_type = FLOCK_NONE;
        }
    }
    // (2) Release exclusive lock
    else {
        flock->type = FLOCK_NONE;
    }

    KERN_ASSERT(flock->type != FLOCK_EX);

    // Remove current process as a lock holder
    flock->lock_holder[curid] = 0;

    // Determine what waiting requests could be granted now
    if(flock->type == FLOCK_NONE){
        // (1)Any waiting requests could be satisfied
        CV_signal(&flock->cv_shared_flock);
        CV_signal(&flock->cv_exclusive_flock);
    } else if (flock->type == FLOCK_SH){
        // (2) Only waiting requests for shared locks could be satisfied
        CV_signal(&flock->cv_shared_flock);
    }
}

