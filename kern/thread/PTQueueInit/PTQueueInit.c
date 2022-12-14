#include "lib/x86.h"
#include <lib/debug.h>

#include "import.h"

int tqueue_contains(unsigned int chid, unsigned int pid);

/**
 * Initializes all the thread queues with tqueue_init_at_id.
 */
void tqueue_init(unsigned int mbi_addr)
{
    unsigned int cpu_idx, chid;
    tcb_init(mbi_addr);

    chid = 0;
    while (chid < NUM_IDS + NUM_CPUS) {
        tqueue_init_at_id(chid);
        chid++;
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
    if(tqueue_contains(chid, pid) > 0){
        return;
    }

    unsigned int tail = tqueue_get_tail(chid);
    // KERN_DEBUG("tail of queue is %d\n", tail);

    if (tail == NUM_IDS) {
        tcb_set_prev(pid, NUM_IDS);
        tcb_set_next(pid, NUM_IDS);
        tqueue_set_head(chid, pid);
        tqueue_set_tail(chid, pid);
        // KERN_DEBUG("head of queue is %d\n", pid);
    } else {
        tcb_set_next(tail, pid);
        tcb_set_prev(pid, tail);
        tcb_set_next(pid, NUM_IDS);
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
    unsigned int head, next, pid;

    pid = NUM_IDS;
    head = tqueue_get_head(chid);
    // KERN_DEBUG("head of queue is %d\n", head);

    if (head != NUM_IDS) {
        pid = head;
        next = tcb_get_next(head);

        if (next == NUM_IDS) {
            tqueue_set_head(chid, NUM_IDS);
            tqueue_set_tail(chid, NUM_IDS);
            // KERN_DEBUG("head of queue is %d\n", NUM_IDS);
        } else {
            tcb_set_prev(next, NUM_IDS);
            tqueue_set_head(chid, next);
            // KERN_DEBUG("head of queue is %d\n", next);
        }
        tcb_set_prev(pid, NUM_IDS);
        tcb_set_next(pid, NUM_IDS);
    }

    return pid;
}

/**
 * Removes the TCB #pid from the queue #chid.
 * Hint: there are many cases in this function.
 */
void tqueue_remove(unsigned int chid, unsigned int pid)
{
    unsigned int prev, next;

    prev = tcb_get_prev(pid);
    next = tcb_get_next(pid);

    if (prev == NUM_IDS) {
        if (next == NUM_IDS) {
            tqueue_set_head(chid, NUM_IDS);
            tqueue_set_tail(chid, NUM_IDS);
        } else {
            tcb_set_prev(next, NUM_IDS);
            tqueue_set_head(chid, next);
        }
    } else {
        if (next == NUM_IDS) {
            tcb_set_next(prev, NUM_IDS);
            tqueue_set_tail(chid, prev);
        } else {
            if (prev != next)
                tcb_set_next(prev, next);
            tcb_set_prev(next, prev);
        }
    }
    tcb_set_prev(pid, NUM_IDS);
    tcb_set_next(pid, NUM_IDS);
}


void tqueue_print(unsigned int chid)
{
    unsigned int curr = tqueue_get_head(chid);
    dprintf("ready queue: ");

    while(curr != NUM_IDS){
        dprintf("%d ", curr),
        curr = tcb_get_next(curr);
    }
    dprintf("\n");
}

int tqueue_contains(unsigned int chid, unsigned int pid)
{
    unsigned int curr = tqueue_get_head(chid);

    while(curr != NUM_IDS){
        if(curr == pid){
            return 1;
        }
        curr = tcb_get_next(curr);
    }

    return 0;
}