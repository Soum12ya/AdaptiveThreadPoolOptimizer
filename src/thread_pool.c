// src/thread_pool.c
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "thread_pool.h"

static void *worker(void *arg) {
    thread_pool_t *pool = (thread_pool_t *)arg;
    while (1) {
        pthread_mutex_lock(&pool->job_queue.lock);
        while (pool->job_queue.front == NULL && !pool->stop) {
            pthread_cond_wait(&pool->job_queue.has_jobs, &pool->job_queue.lock);
        }
        if (pool->stop && pool->job_queue.front == NULL) {
            pthread_mutex_unlock(&pool->job_queue.lock);
            break;
        }
        job_t *job = pool->job_queue.front;
        if (job) {
            pool->job_queue.front = job->next;
            if (pool->job_queue.front == NULL) pool->job_queue.rear = NULL;
            pool->job_queue.size--;
        }
        pthread_mutex_unlock(&pool->job_queue.lock);
        if (job) {
            job->function(job->arg);
            free(job);
        }
    }
    return NULL;
}
void init_thread_pool(thread_pool_t *pool, int num_threads, int max_threads) {
    pool->num_threads = num_threads;
    pool->max_threads = max_threads;
    pool->stop = 0;
    init_job_queue(&pool->job_queue);
    pool->threads = malloc(sizeof(pthread_t) * max_threads);
    for (int i = 0; i < num_threads; ++i) {
        pthread_create(&pool->threads[i], NULL, worker, pool);
    }
}
void submit_job(thread_pool_t *pool, void (*function)(void *), void *arg) {
    enqueue_job(&pool->job_queue, function, arg);
}
void destroy_thread_pool(thread_pool_t *pool) {
    pthread_mutex_lock(&pool->job_queue.lock);
    pool->stop = 1;
    pthread_cond_broadcast(&pool->job_queue.has_jobs);
    pthread_mutex_unlock(&pool->job_queue.lock);
    for (int i = 0; i < pool->num_threads; ++i) {
        pthread_join(pool->threads[i], NULL);
    }
    free(pool->threads);
}
void scale_thread_pool(thread_pool_t *pool, int new_size) {
    if (new_size > pool->max_threads) new_size = pool->max_threads;
    if (new_size <= pool->num_threads) return;
    for (int i = pool->num_threads; i < new_size; ++i) {
        pthread_create(&pool->threads[i], NULL, worker, pool);
    }
    printf("[Scaler] Scaled thread pool from %d to %d threads.\n", pool->num_threads, new_size);
    pool->num_threads = new_size;
}
