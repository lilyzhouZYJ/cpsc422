#include <lib/x86.h>
#include <lib/thread.h>
#include <lib/spinlock.h>
#include <lib/debug.h>
#include <dev/lapic.h>
#include <pcpu/PCPUIntro/export.h>

#include "import.h"

// Spinlock
spinlock_t thread_lk;

void thread_spinlock_init(void) {
    spinlock_init(&thread_lk);
}

void thread_lock(void) {
    spinlock_acquire(&thread_lk);
}

void thread_unlock(void) {
    spinlock_release(&thread_lk);
}

unsigned int counters[NUM_CPUS];



void thread_init(unsigned int mbi_addr)
{
	thread_spinlock_init();
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
	thread_lock();
    unsigned int pid = kctx_new(entry, id, quota);
    if (pid != NUM_IDS) {
        tcb_set_cpu(pid, get_pcpu_idx());
        tcb_set_state(pid, TSTATE_READY);
        tqueue_enqueue(NUM_IDS + get_pcpu_idx(), pid);
    }

	thread_unlock();
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
	thread_lock();
    counters[get_pcpu_idx()] = 0;

    unsigned int new_cur_pid;
    unsigned int old_cur_pid = get_curid();

    tcb_set_state(old_cur_pid, TSTATE_READY);
    tqueue_enqueue(NUM_IDS + get_pcpu_idx(), old_cur_pid);

    new_cur_pid = tqueue_dequeue(NUM_IDS + get_pcpu_idx());
    tcb_set_state(new_cur_pid, TSTATE_RUN);
    set_curid(new_cur_pid);

    if (old_cur_pid != new_cur_pid) {
        thread_unlock();
        kctx_switch(old_cur_pid, new_cur_pid);
    } else {
        thread_unlock();
    }
}

// Records (for each CPU) the number of miliseconds that has elapsed since the last thread switch. 
// Once that reaches SCHED_SLICE miliseconds, you should yield the current thread to other ready 
// thread by calling thread_yield, and reset the counter.
void sched_update(){
    // There are LAPIC_TIMER_INTR_FREQ = 1000 interrupts per second;
    // which means 1 interrupt per millisecond
    thread_lock();
    unsigned int cpu_idx = get_pcpu_idx();
    counters[cpu_idx]++;
    if(counters[cpu_idx] > SCHED_SLICE){
        // Reaches SCHED_SLICE milliseconds
        counters[cpu_idx] = 0;
        thread_unlock();
        thread_yield();
    } else {
        thread_unlock();
    }
}
