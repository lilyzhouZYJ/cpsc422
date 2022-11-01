#include <lib/spinlock.h>
#include <lib/debug.h>
#include <dev/intr.h>
#include "import.h"

struct TQueue {
    unsigned int head;
    unsigned int tail;
};

struct CV {
    struct TQueue queue;
};

#define QUEUE_SIZE 2

int BBQueue[QUEUE_SIZE];
int head;
int tail; // points to next empty slot
// Note: head and tail should NEVER be out of bounds (always assign with %QUEUE_SIZE)
int size;

struct CV element_added_cv;
struct CV element_removed_cv;

spinlock_t queue_lk;

void BBQ_init()
{
    head = 0;
    tail = 0;
    size = 0;
    CV_init(&element_added_cv);
    CV_init(&element_removed_cv);

    spinlock_init(&queue_lk);
}

void BBQ_insert(int i)
{
    // Acquire lock
    // intr_local_disable();
    spinlock_acquire(&queue_lk);

    // Check if queue is full
    while(size == QUEUE_SIZE){
        // Queue is full
        CV_wait(&element_removed_cv, &queue_lk);
    } 

    // Enqueue to tail
    BBQueue[tail] = i;
    tail = (tail + 1) % QUEUE_SIZE;
    size++;

    // Signal to element_added_cv
	CV_signal(&element_added_cv);

    KERN_DEBUG("CPU %d: Process %d: Produced %d\n", get_pcpu_idx(), get_curid(), i);

    // Release lock
    spinlock_release(&queue_lk);
    // intr_local_enable();
}

int BBQ_remove()
{
    // Acquire lock
    // intr_local_disable();
    spinlock_acquire(&queue_lk);

    // Check if queue is empty
    while(size == 0){
        CV_wait(&element_added_cv, &queue_lk);
    }

    // Pop from front
    int result = BBQueue[head];
    head = (head + 1) % QUEUE_SIZE;
    size--;

    // Signal to element_removed_cv
	CV_signal(&element_removed_cv);

    KERN_DEBUG("CPU %d: Process %d: Consumed %d\n", get_pcpu_idx(), get_curid(), result);

    // Release lock
    spinlock_release(&queue_lk);
    // intr_local_enable();

	return result;
}