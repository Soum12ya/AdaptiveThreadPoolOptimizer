// include/thread_pool.h
#ifndef THREAD_POOL_H
#define THREAD_POOL_H
#include <pthread.h>
#include "job_queue.h"
typedef struct thread_pool {
    pthread_t *threads;
    int num_threads;
    int max_threads;
    job_queue_t job_queue;
    int stop;
} thread_pool_t;
void init_thread_pool(thread_pool_t *pool, int num_threads, int max_threads);
void submit_job(thread_pool_t *pool, void (*function)(void *), void *arg);
void destroy_thread_pool(thread_pool_t *pool);
void scale_thread_pool(thread_pool_t *pool, int new_size);
#endif
