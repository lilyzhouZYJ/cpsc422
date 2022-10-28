#include "import.h"

struct CV {
    CVThreadQueue queue;
};

void CV_init(struct CV cv)
{
    cv_queue_init(cv.queue);    
}

void CV_wait(struct CV cv, spinlock_t *lk)
{
    // (1) Release lock
    spinlock_release(lk);

    // (2) Add TCB of current thread to waiting queue
    unsigned int curid = get_curid();
	cv_queue_enqueue(CV.queue, curid);

	// (3) Switch to new thread
	thread_yield();

	// (4) Re-acquire lock
	spinlock_acquire(lk);
}

void CV_signal(struct CV cv) {
    // Check if waiting queue is empty
    if(cv_queue_is_empty(cv.queue) == 0){
        // Not empty: add new thread TCB to ready list
        new_thread_id = cv_queue_dequeue(CV_Queue);
        // Add to ready list
        tqueue_enqueue(NUM_IDS + get_pcpu_idx(), new_thread_id);
    }
}