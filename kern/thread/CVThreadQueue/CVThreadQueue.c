
/*
struct TCB {
    t_state state;
    unsigned int cpuid;
    unsigned int prev;
    unsigned int next;
} in_cache_line;

struct TCB TCBPool[NUM_IDS];
*/

typedef struct TQueue CVThreadQueue;

void cv_queue_init(CVThreadQueue CV_Queue)
{
    CV_Queue.head = NUM_IDS;
    CV_Queue.tail = NUM_IDS;
}

unsigned int cv_queue_get_head(CVThreadQueue CV_Queue)
{
    return CV_Queue.head;
}

void cv_queue_set_head(CVThreadQueue CV_Queue, unsigned int head)
{
    CV_Queue.head = head;
}

unsigned int cv_queue_get_tail(CVThreadQueue CV_Queue)
{
    return CV_Queue.tail;
}

void cv_queue_set_tail(CVThreadQueue CV_Queue, unsigned int tail)
{
    CV_Queue.tail = tail;
}

unsigned int cv_queue_is_empty(CVThreadQueue CV_Queue)
{
    return CV_Queue.head == NUM_IDS ? 1 : 0;
}

void cv_queue_enqueue(CVThreadQueue CV_Queue, unsigned int pid)
{
    unsigned int tail = cv_queue_get_tail();

    if (tail == NUM_IDS) {
        tcb_set_prev(pid, NUM_IDS);
        tcb_set_next(pid, NUM_IDS);
        cv_queue_set_head(pid);
        cv_queue_set_tail(pid);
    } else {
        tcb_set_next(tail, pid);
        tcb_set_prev(pid, tail);
        tcb_set_next(pid, NUM_IDS);
        cv_queue_set_tail(pid);
    }
}

unsigned int cv_queue_dequeue(CVThreadQueue CV_Queue)
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