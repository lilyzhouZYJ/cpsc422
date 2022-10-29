#ifndef _KERN_THREAD_CONDVARIABLE_H_
#define _KERN_THREAD_CONDVARIABLE_H_

#ifdef _KERN_

struct TQueue;

unsigned int get_curid();
void set_curid(unsigned int curid);
int get_pcpu_idx(void);
void tcb_set_state(unsigned int pid, unsigned int state);
void kctx_switch(unsigned int from_pid, unsigned int to_pid);
unsigned int tqueue_dequeue(unsigned int chid);
void tqueue_enqueue(unsigned int chid, unsigned int pid);

void cv_queue_init(struct TQueue * CV_Queue);
unsigned int cv_queue_get_head(struct TQueue * CV_Queue);
unsigned int cv_queue_get_tail(struct TQueue * CV_Queue);
unsigned int cv_queue_is_empty(struct TQueue * CV_Queue);
void cv_queue_enqueue(struct TQueue * CV_Queue, unsigned int pid);
unsigned int cv_queue_dequeue(struct TQueue * CV_Queue);

#endif  /* _KERN_ */

#endif  /* !_KERN_THREAD_CONDVARIABLE_H_ */
