#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "thread_pool.h"
#include "resource_monitor.h"
#include "http_server.h" // For extern globals and http_server_thread

// Simulate atomic job completion count
int jobs_completed = 0;

void sample_job(void *arg) {
    int id = *(int *)arg;
    printf("[Thread] Executing job %d\n", id);
    sleep(1);
    printf("[Thread] Finished job %d\n", id);
    free(arg);
    __sync_fetch_and_add(&jobs_completed, 1); // atomic increment
}

// ML-based thread count prediction (from your model)
int predict_thread_count(float cpu_usage, float mem_free, float queue_size, float throughput) {
    if (queue_size <= 6.5) {
        if (throughput <= 100.5) {
            if (cpu_usage <= 70.0) {
                return 4;
            } else {
                return 6;
            }
        } else {
            return 8;
        }
    } else {
        return 10;
    }
}

int main() {
    // Start HTTP server in background thread
    pthread_t http_tid;
    pthread_create(&http_tid, NULL, http_server_thread, NULL);

    thread_pool_t pool;
    init_thread_pool(&pool, 3, 10);  // start with 3, max 10 threads

    // Prompt user for number of jobs
    int num_jobs = 0;
    printf("Enter the number of jobs to submit: ");
    fflush(stdout);
    scanf("%d", &num_jobs);

    // Submit the requested number of jobs
    for (int i = 0; i < num_jobs; ++i) {
        int *job_id = malloc(sizeof(int));
        *job_id = i + 1;
        submit_job(&pool, sample_job, job_id);
    }

    int prev_completed = 0;
    while (jobs_completed < num_jobs) {
        system_metrics_t metrics = get_system_metrics();
        print_system_metrics(metrics);
        log_system_metrics(metrics);

        int qsize = get_queue_size(&pool.job_queue);
        int threads = pool.num_threads;
        int completed_now = jobs_completed;
        float throughput = completed_now - prev_completed;
        prev_completed = completed_now;

        // Update global metrics for HTTP server
        g_cpu = metrics.cpu_usage;
        g_mem = metrics.mem_free_percent;
        g_queue = qsize;
        g_threads = threads;
        g_throughput = throughput;

        // Use ML-based prediction for scaling
        int ml_threads = predict_thread_count(metrics.cpu_usage, metrics.mem_free_percent, (float)qsize, throughput);
        if (ml_threads != pool.num_threads && ml_threads <= pool.max_threads && ml_threads > 0) {
            scale_thread_pool(&pool, ml_threads);
            printf("[Scaler] (ML) Scaled thread pool to %d threads.\n", ml_threads);
        }

        sleep(1);
    }

    destroy_thread_pool(&pool);
    printf("All jobs processed. Exiting.\n");

    // Optionally, terminate HTTP server here if you add logic for it
    // For now, just exit the program (which will end the server thread)
    return 0;
}
