void CV_init(struct CV cv);
void CV_wait(struct CV cv, spinlock_t *lk);
void CV_signal(struct CV cv);