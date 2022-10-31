#include <lib/spinlock.h>
#include <lib/thread.h>
#include <dev/intr.h>
#include "import.h"

struct TQueue {
    unsigned int head;
    unsigned int tail;
};

struct CV {
    struct TQueue queue;
};

// Lock for wait and signal
spinlock_t cond_var_lk;

void CV_init(struct CV *cv)
{
    cv_queue_init(&(cv->queue));
    spinlock_init(&cond_var_lk); 
}

void CV_wait(struct CV *cv, spinlock_t *lk)
{
    // Disable interrupts
    // intr_local_disable();

    spinlock_acquire(&cond_var_lk); // make sure wait is atomic

    // (1) Release lock
    spinlock_release(lk);

    // (2) Add TCB of current thread to waiting queue
    unsigned int curid = get_curid();
	cv_queue_enqueue(&(cv->queue), curid);

	// (3) Switch to new thread
    // 3.a. Set current thread to READY
    tcb_set_state(curid, TSTATE_READY);
    // 3.b. Wake up a new ready thread of current CPU and set it to run
    unsigned int new_pid = tqueue_dequeue(NUM_IDS + get_pcpu_idx());
    tcb_set_state(new_pid, TSTATE_RUN);
    set_curid(new_pid);
    // 3.c. context switch
    spinlock_release(&cond_var_lk); // release cond_var_lk
    if (curid != new_pid) {
        // intr_local_enable();
        kctx_switch(curid, new_pid);
    }

	// (4) Re-acquire lock
	spinlock_acquire(lk);

    // Enable interrupts
    // intr_local_enable();
}

void CV_signal(struct CV *cv) {
    // Disable interrupts
    // intr_local_disable();

    spinlock_acquire(&cond_var_lk);

    // Check if waiting queue is empty
    if(cv_queue_is_empty(&(cv->queue)) == 0){
        // Not empty: add new thread TCB to ready list of its original CPU
        unsigned int new_thread_id = cv_queue_dequeue(&(cv->queue));
        tqueue_enqueue(NUM_IDS + tcb_get_cpu(new_thread_id), new_thread_id);
    }

    spinlock_release(&cond_var_lk);

    // Enable interrupts
    // intr_local_enable();
}