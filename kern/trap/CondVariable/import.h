
unsigned int get_curid();

void cv_queue_init(CVThreadQueue CV_Queue);
unsigned int cv_queue_get_head(CVThreadQueue CV_Queue);
unsigned int cv_queue_get_tail(CVThreadQueue CV_Queue);
unsigned int cv_queue_is_empty(CVThreadQueue CV_Queue);
void cv_queue_enqueue(CVThreadQueue CV_Queue, unsigned int pid);
unsigned int cv_queue_dequeue(CVThreadQueue CV_Queue);