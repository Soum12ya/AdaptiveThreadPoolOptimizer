// include/job_queue.h
#ifndef JOB_QUEUE_H
#define JOB_QUEUE_H
#include <pthread.h>
typedef struct job {
    void (*function)(void *arg);
    void *arg;
    struct job *next;
} job_t;
typedef struct job_queue {
    job_t *front;
    job_t *rear;
    pthread_mutex_t lock;
    pthread_cond_t has_jobs;
    int size;
} job_queue_t;
void init_job_queue(job_queue_t *queue);
void enqueue_job(job_queue_t *queue, void (*function)(void *), void *arg);
job_t *dequeue_job(job_queue_t *queue);
int get_queue_size(job_queue_t *queue);
#endif
