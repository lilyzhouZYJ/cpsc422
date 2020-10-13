#include <lib/cv.h>
#include <lib/debug.h>
#include <lib/string.h>
#include <lib/thread.h>
#include <dev/intr.h>
#include <thread/PCurID/export.h>
#include <thread/PThread/export.h>

void CV_init(CV *cv)
{
    int i;
    cv->tail = 0;
    for (i = 0; i < NUM_IDS; i++) {
        cv->queue[i] = 0;
    }
}

static void CV_enqueue(CV *cv, unsigned int pid)
{
    NO_INTR(
        KERN_ASSERT(0 < pid && pid < NUM_IDS);
        KERN_ASSERT(0 <= cv->tail && cv->tail < NUM_IDS);
        KERN_ASSERT(cv->queue[cv->tail] == 0);
        KERN_ASSERT(cv->queue[pid] == 0)
    );

    cv->queue[cv->tail] = pid;
    cv->tail = pid;
}

static unsigned int CV_dequeue(CV *cv)
{
    unsigned int pid = cv->queue[0];

    if (cv->queue[pid] == 0) {
        cv->tail = 0;
    }
    cv->queue[0] = cv->queue[pid];
    cv->queue[pid] = 0;

    return pid;
}

void CV_wait(CV *cv, spinlock_t *lk)
{
    unsigned int old_cur_pid;
    NO_INTR(
        KERN_ASSERT(spinlock_holding(lk))
    );

    old_cur_pid = get_curid();
    CV_enqueue(cv, old_cur_pid);

    NO_INTR(
        thread_suspend(lk, old_cur_pid)
    );

    spinlock_acquire(lk);
}

void CV_signal(CV *cv)
{
    unsigned int pid = CV_dequeue(cv);
    if (pid != 0) {
        NO_INTR(
            thread_ready(pid)
        );
    }
}

void CV_broadcast(CV *cv)
{
    unsigned int pid;

    while ((pid = CV_dequeue(cv)) != 0) {
        NO_INTR(
            thread_ready(pid)
        );
    }
}

void BB_init(BoundedBuffer *bb)
{
    memzero(bb->buf, BUFFER_SIZE);
    bb->head = bb->size = 0;
    spinlock_init(&bb->lk);
    CV_init(&bb->empty);
    CV_init(&bb->full);
}

bool BB_is_empty(BoundedBuffer *bb)
{
    return bb->size == 0;
}

bool BB_is_full(BoundedBuffer *bb)
{
    return bb->size == BUFFER_SIZE;
}

void BB_enqueue(BoundedBuffer *bb, unsigned int val)
{
    unsigned int idx;
    spinlock_acquire(&bb->lk);

    while (BB_is_full(bb)) {
        CV_wait(&bb->full, &bb->lk);
    }

    idx = (bb->head + bb->size) % BUFFER_SIZE;
    bb->buf[idx] = val;
    bb->size++;

    CV_signal(&bb->empty);
    spinlock_release(&bb->lk);
}

unsigned int BB_dequeue(BoundedBuffer *bb)
{
    unsigned int val;
    spinlock_acquire(&bb->lk);

    while (BB_is_empty(bb)) {
        CV_wait(&bb->empty, &bb->lk);
    }

    val = bb->buf[bb->head];
    bb->head = (bb->head + 1) % BUFFER_SIZE;
    bb->size--;

    CV_signal(&bb->full);
    spinlock_release(&bb->lk);
    return val;
}
