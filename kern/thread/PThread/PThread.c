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
    unsigned int pid = kctx_new(entry, id, quota);
    if (pid != NUM_IDS) {
        tcb_set_state(pid, TSTATE_READY);
        tqueue_enqueue(NUM_IDS, pid);
    }

    return pid;
}

unsigned int thread_fork(void *entry, unsigned int id)
{
    unsigned int quota = (container_get_quota(id) - container_get_usage(id)) / 2;
    unsigned int pid = kctx_new(entry, id, quota);
    if (pid != NUM_IDS) {
        map_cow(id, pid);
        tcb_set_state(pid, TSTATE_READY);
        tqueue_enqueue(NUM_IDS, pid);
    }

    return pid;
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
    unsigned int new_cur_pid;
    unsigned int old_cur_pid = get_curid();

    tcb_set_state(old_cur_pid, TSTATE_READY);
    tqueue_enqueue(NUM_IDS, old_cur_pid);

    new_cur_pid = tqueue_dequeue(NUM_IDS);
    tcb_set_state(new_cur_pid, TSTATE_RUN);
    set_curid(new_cur_pid);

    if (old_cur_pid != new_cur_pid) {
        kctx_switch(old_cur_pid, new_cur_pid);
    }
}
