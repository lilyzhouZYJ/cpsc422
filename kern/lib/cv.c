#include <lib/spinlock.h>
#include <lib/thread.h>
#include <dev/intr.h>
#include <thread/PThread/export.h>
#include "cv.h"

void CV_init(struct CV *cv)
{
    cv_queue_init(&(cv->queue));
}

void CV_wait(struct CV *cv, spinlock_t *lk)
{
    // (1) Add TCB of current thread to waiting queue
    unsigned int curid = get_curid();
	cv_queue_enqueue(&(cv->queue), curid);

    // (2) Release BBQueue lock: this lock protects CV + BBQueue
    spinlock_release(lk);

	// (3) Switch to new thread
    thread_suspend();

	// (4) Re-acquire lock
	spinlock_acquire(lk);
}

void CV_signal(struct CV *cv) {
    // Check if waiting queue is empty
    if(cv_queue_is_empty(&(cv->queue)) == 0){
        // Not empty: add new thread TCB to ready list of its original CPU
        unsigned int new_thread_id = cv_queue_dequeue(&(cv->queue));
        thread_resume(new_thread_id);
    }
}