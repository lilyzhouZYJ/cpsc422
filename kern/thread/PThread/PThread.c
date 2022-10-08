#include <lib/x86.h>
#include <lib/thread.h>

#include "import.h"

void thread_init(unsigned int mbi_addr)
{
    tqueue_init(mbi_addr);
    set_curid(0);
    tcb_set_state(0, TSTATE_RUN);
}

/**
 * Allocates a new child thread context, sets the state of the new child thread
 * to ready, and pushes it to the ready queue.
 * It returns the child thread id.
 */
unsigned int thread_spawn(void *entry, unsigned int id, unsigned int quota)
{
    // TODO
	// Allocate a new child thread
	unsigned int child_pid = kctx_new(entry, id, quota);
	if(child_pid == NUM_IDS){
		return NUM_IDS;
	}

	// Set child thread to ready
	tcb_set_state(child_pid, TSTATE_READY);

	// Push child to the ready queue
	tqueue_enqueue(NUM_IDS, child_pid);

    return child_pid;
}

/**
 * Yield to the next thread in the ready queue.
 * You should set the currently running thread state as ready,
 * and push it back to the ready queue.
 * Then set the state of the popped thread as running, set the
 * current thread id, and switch to the new kernel context.
 * Hint: If you are the only thread that is ready to run,
 * do you need to switch to yourself?
 */
void thread_yield(void)
{
    // TODO
	// Set curr thread to ready and push it to ready queue
	unsigned int curr_pid = get_curid();
	tcb_set_state(curr_pid, TSTATE_READY);
	tqueue_enqueue(NUM_IDS, curr_pid);
	
	// Pop next thread from ready queue
	unsigned int next_thread_pid = tqueue_dequeue(NUM_IDS);
	// Set it to running
	tcb_set_state(next_thread_pid, TSTATE_RUN);
	
	// Set current thread ID
	set_curid(next_thread_pid);

	// Switch to new kernel context
	if(next_thread_pid != curr_pid){
		kctx_switch(curr_pid, next_thread_pid);
	}
}
