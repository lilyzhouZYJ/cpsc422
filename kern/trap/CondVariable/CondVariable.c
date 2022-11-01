#include <lib/spinlock.h>
#include <lib/thread.h>
#include <dev/intr.h>
#include <thread/PThread/export.h>
#include "import.h"

struct TQueue {
    unsigned int head;
    unsigned int tail;
};

struct CV {
    struct TQueue queue;
};

extern spinlock_t thread_lk;

void CV_init(struct CV *cv)
{
    cv_queue_init(&(cv->queue));
}

void CV_wait(struct CV *cv, spinlock_t *lk)
{
    // (1) Add TCB of current thread to waiting queue
    unsigned int curid = get_curid();
	cv_queue_enqueue(&(cv->queue), curid);

    // (2) Release BBQueue lock: this lock needs to protect CV
    spinlock_release(lk);

	// (3) Switch to new thread
    // 3.a Acquire thread lock: this lock protects the ready queue
    spinlock_acquire(&thread_lk);
    // 3.b. Set current thread to SLEEP
    tcb_set_state(curid, TSTATE_SLEEP);
    // 3.c. Wake up a new ready thread of current CPU and set it to run
    unsigned int new_pid = tqueue_dequeue(NUM_IDS + get_pcpu_idx());
    tcb_set_state(new_pid, TSTATE_RUN);
    set_curid(new_pid);
    // 3.d. context switch
    spinlock_release(&thread_lk);
    kctx_switch(curid, new_pid);

	// (4) Re-acquire lock
	spinlock_acquire(lk);
}

void CV_signal(struct CV *cv) {
    // Check if waiting queue is empty
    if(cv_queue_is_empty(&(cv->queue)) == 0){
        // Not empty: add new thread TCB to ready list of its original CPU
        unsigned int new_thread_id = cv_queue_dequeue(&(cv->queue));

        spinlock_acquire(&thread_lk);

        tcb_set_state(new_thread_id, TSTATE_READY);
        tqueue_enqueue(NUM_IDS + tcb_get_cpu(new_thread_id), new_thread_id);

        spinlock_release(&thread_lk);
    }
}