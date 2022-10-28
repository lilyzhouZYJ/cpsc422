
#include "import.h"

const int QUEUE_SIZE = 5;

struct BBQueue {
    int queue[QUEUE_SIZE];
    int head;
    int tail;

    struct CV element_added_cv;
    struct CV element_removed_cv;

    spinlock_t queue_lk;
};

struct BBQueue bbq;

void BBQ_init()
{
    head = 0;
    tail = 0;
    CV_init(element_added_cv);
    CV_init(element_removed_cv);

    spinlock_init(&(bbq.queue_lk));
}

void BBQ_enqueue(int n)
{
    // Acquire lock
    spinlock_acquire(&(queue_lk));

    // Enqueue to tail
    int target_index = tail / QUEUE_SIZE;
    queue[target_index] = n;
    tail = target_index + 1;

    

    // Release lock
    spinlock_release(&(queue_lk));
}