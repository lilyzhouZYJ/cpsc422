#include <lib/spinlock.h>
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

struct CV element_added_cv;
struct CV element_removed_cv;

spinlock_t queue_lk;

void BBQ_init()
{
    head = 0;
    tail = 0;
    CV_init(&element_added_cv);
    CV_init(&element_removed_cv);

    spinlock_init(&queue_lk);
}

void BBQ_insert(int n)
{
    // Acquire lock
    spinlock_acquire(&queue_lk);

    // Check if queue is full
    while(head == (tail + 1) % QUEUE_SIZE){
        // Queue is full
        CV_wait(&element_removed_cv, &queue_lk);
    } 

    // Enqueue to tail
    BBQueue[tail] = n;
    tail = (tail + 1) % QUEUE_SIZE;

    // Signal to element_added_cv
	CV_signal(&element_added_cv);

    // Release lock
    spinlock_release(&queue_lk);
}

int BBQ_remove()
{
    // Acquire lock
    spinlock_acquire(&queue_lk);

    // Check if queue is empty
    while(head == tail){
        CV_wait(&element_added_cv, &queue_lk);
    }

    // Pop from front
    int result = BBQueue[head];
    head = (head + 1) % QUEUE_SIZE;

    // Signal to element_removed_cv
	CV_signal(&element_removed_cv);
	
    // Release lock
    spinlock_release(&queue_lk);

	return result;
}