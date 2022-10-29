#include <lib/x86.h>
#include "import.h"

struct TQueue {
    unsigned int head;
    unsigned int tail;
};

void cv_queue_init(struct TQueue * CV_Queue)
{
    CV_Queue->head = NUM_IDS;
    CV_Queue->tail = NUM_IDS;
}

unsigned int cv_queue_get_head(struct TQueue * CV_Queue)
{
    return CV_Queue->head;
}

void cv_queue_set_head(struct TQueue * CV_Queue, unsigned int head)
{
    CV_Queue->head = head;
}

unsigned int cv_queue_get_tail(struct TQueue * CV_Queue)
{
    return CV_Queue->tail;
}

void cv_queue_set_tail(struct TQueue * CV_Queue, unsigned int tail)
{
    CV_Queue->tail = tail;
}

unsigned int cv_queue_is_empty(struct TQueue * CV_Queue)
{
    return CV_Queue->head == NUM_IDS ? 1 : 0;
}

void cv_queue_enqueue(struct TQueue * CV_Queue, unsigned int pid)
{
    unsigned int tail = cv_queue_get_tail(CV_Queue);

    if (tail == NUM_IDS) {
        tcb_set_prev(pid, NUM_IDS);
        tcb_set_next(pid, NUM_IDS);
        cv_queue_set_head(CV_Queue, pid);
        cv_queue_set_tail(CV_Queue, pid);
    } else {
        tcb_set_next(tail, pid);
        tcb_set_prev(pid, tail);
        tcb_set_next(pid, NUM_IDS);
        cv_queue_set_tail(CV_Queue, pid);
    }
}

unsigned int cv_queue_dequeue(struct TQueue * CV_Queue)
{
    unsigned int head, next, pid;

    pid = NUM_IDS;
    head = cv_queue_get_head(CV_Queue);

    if (head != NUM_IDS) {
        pid = head;
        next = tcb_get_next(head);

        if (next == NUM_IDS) {
            cv_queue_set_head(CV_Queue, NUM_IDS);
            cv_queue_set_tail(CV_Queue, NUM_IDS);
        } else {
            tcb_set_prev(next, NUM_IDS);
            cv_queue_set_head(CV_Queue, next);
        }
        tcb_set_prev(pid, NUM_IDS);
        tcb_set_next(pid, NUM_IDS);
    }

    return pid;
}