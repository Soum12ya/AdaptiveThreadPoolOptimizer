// src/job_queue.c
#include <stdlib.h>
#include <pthread.h>
#include "job_queue.h"

void init_job_queue(job_queue_t *queue) {
    queue->front = queue->rear = NULL;
    queue->size = 0;
    pthread_mutex_init(&queue->lock, NULL);
    pthread_cond_init(&queue->has_jobs, NULL);
}
void enqueue_job(job_queue_t *queue, void (*function)(void *), void *arg) {
    job_t *new_job = malloc(sizeof(job_t));
    new_job->function = function;
    new_job->arg = arg;
    new_job->next = NULL;
    pthread_mutex_lock(&queue->lock);
    if (queue->rear) {
        queue->rear->next = new_job;
        queue->rear = new_job;
    } else {
        queue->front = queue->rear = new_job;
    }
    queue->size++;
    pthread_cond_signal(&queue->has_jobs);
    pthread_mutex_unlock(&queue->lock);
}
job_t *dequeue_job(job_queue_t *queue) {
    job_t *job = NULL;
    pthread_mutex_lock(&queue->lock);
    while (queue->front == NULL) {
        pthread_cond_wait(&queue->has_jobs, &queue->lock);
    }
    job = queue->front;
    queue->front = job->next;
    if (queue->front == NULL) queue->rear = NULL;
    queue->size--;
    pthread_mutex_unlock(&queue->lock);
    return job;
}
int get_queue_size(job_queue_t *queue) {
    pthread_mutex_lock(&queue->lock);
    int size = queue->size;
    pthread_mutex_unlock(&queue->lock);
    return size;
}
