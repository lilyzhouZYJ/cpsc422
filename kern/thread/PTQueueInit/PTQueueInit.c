#include "lib/x86.h"

#include "import.h"

/**
 * Initializes all the thread queues with tqueue_init_at_id.
 */
void tqueue_init(unsigned int mbi_addr)
{
    // TODO: define your local variables here.

    tcb_init(mbi_addr);

    // TODO
	for(unsigned int chid = 0; chid <= NUM_IDS; chid++){
		tqueue_init_at_id(chid);
	}
}

/**
 * Insert the TCB #pid into the tail of the thread queue #chid.
 * Recall that the doubly linked list is index based.
 * So you only need to insert the index.
 * Hint: there are multiple cases in this function.
 */
void tqueue_enqueue(unsigned int chid, unsigned int pid)
{
    // TODO
	unsigned int tail_pid = tqueue_get_tail(chid); // get ID of the tail of thread queue #chid

	if(tail_pid == NUM_IDS){
		// (1) Queue is empty: set both head and tail to the TCB #pid
		tqueue_set_head(chid, pid);
		tqueue_set_tail(chid, pid);
	} else {
		// (2) Queue is not empty:
		tcb_set_next(tail_pid, pid);
		tcb_set_prev(pid, tail_pid);
		tqueue_set_tail(chid, pid);
	}
}

/**
 * Reverse action of tqueue_enqueue, i.e. pops a TCB from the head of the specified queue.
 * It returns the popped thread's id, or NUM_IDS if the queue is empty.
 * Hint: there are multiple cases in this function.
 */
unsigned int tqueue_dequeue(unsigned int chid)
{
    // TODO
	unsigned int head_pid = tqueue_get_head(chid);
	if(head_pid == NUM_IDS){
		// (1) Queue is empty
		return NUM_IDS;
	} else {
		unsigned int next_pid = tcb_get_next(head_pid);
		// Reset head
		tqueue_set_head(chid, next_pid);
		// Reset the next pointer
		tcb_set_next(head_pid, NUM_IDS);

		// Check if the head is also the tail
		unsigned int tail_pid = tqueue_get_tail(chid);
		if(tail_pid == head_pid){
			// Reset tail
			tqueue_set_tail(chid, NUM_IDS);
		}

		// Return
		return head_pid;
	}
}

/**
 * Removes the TCB #pid from the queue #chid.
 * Hint: there are many cases in this function.
 */
void tqueue_remove(unsigned int chid, unsigned int pid)
{
    // TODO
	unsigned int prev_pid = tcb_get_prev(pid);
	unsigned int next_pid = tcb_get_next(pid);

	// Reset prev and next pointers
	tcb_set_prev(pid, NUM_IDS);
	tcb_set_next(pid, NUM_IDS);

	if(prev_pid == NUM_IDS){
		// TCB #pid is head; reset head
		tqueue_set_head(chid, next_pid);
	} else {
		// TCB #pid is not head
		tcb_set_next(prev_pid, next_pid);
	}

	if(next_pid == NUM_IDS){
		// TCB #pid is tail; reset tail
		tqueue_set_tail(chid, prev_pid);
	} else {
		// TCB #pid is not tail
		tcb_set_prev(next_pid, prev_pid);
	}
}
