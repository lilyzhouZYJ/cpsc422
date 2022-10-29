#ifndef _KERN_THREAD_CVTHREADQUEUE_H_
#define _KERN_THREAD_CVTHREADQUEUE_H_

#ifdef _KERN_

void cv_queue_init(struct TQueue * CV_Queue);
unsigned int cv_queue_get_head(struct TQueue * CV_Queue);
unsigned int cv_queue_get_tail(struct TQueue * CV_Queue);
unsigned int cv_queue_is_empty(struct TQueue * CV_Queue);
void cv_queue_enqueue(struct TQueue * CV_Queue, unsigned int pid);
unsigned int cv_queue_dequeue(struct TQueue * CV_Queue);

#endif  /* _KERN_ */

#endif  /* !_KERN_THREAD_CVTHREADQUEUE_H_ */
