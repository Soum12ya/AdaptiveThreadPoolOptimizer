#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "resource_monitor.h"


// Improved memory usage: MemFree + Buffers + Cached
float calculate_memory_usage() {
    FILE *fp = fopen("/proc/meminfo", "r");
    if (!fp) return -1;

    char label[64];
    unsigned long total = 0, mem_free = 0, buffers = 0, cached = 0;
    while (fscanf(fp, "%s", label) != EOF) {
        if (strcmp(label, "MemTotal:") == 0) fscanf(fp, "%lu", &total);
        else if (strcmp(label, "MemFree:") == 0) fscanf(fp, "%lu", &mem_free);
        else if (strcmp(label, "Buffers:") == 0) fscanf(fp, "%lu", &buffers);
        else if (strcmp(label, "Cached:") == 0) fscanf(fp, "%lu", &cached);
        // Stop early if all found
        if (total && mem_free && buffers && cached) break;
    }
    fclose(fp);

    if (total == 0) return -1;
    float available = mem_free + buffers + cached;
    return (available * 100.0f) / total;
}

float calculate_cpu_usage() {
    FILE *fp = fopen("/proc/stat", "r");
    if (!fp) return -1;

    char line[256];
    fgets(line, sizeof(line), fp);
    fclose(fp);

    unsigned long user, nice, system, idle;
    sscanf(line, "cpu  %lu %lu %lu %lu", &user, &nice, &system, &idle);
    static unsigned long prev_total = 0, prev_idle = 0;

    unsigned long total = user + nice + system + idle;
    unsigned long total_diff = total - prev_total;
    unsigned long idle_diff = idle - prev_idle;

    prev_total = total;
    prev_idle = idle;

    if (total_diff == 0) return 0;
    return 100.0f * (total_diff - idle_diff) / total_diff;
}

system_metrics_t get_system_metrics() {
    system_metrics_t metrics;
    metrics.cpu_usage = calculate_cpu_usage();
    metrics.mem_free_percent = calculate_memory_usage();
    return metrics;
}

void print_system_metrics(system_metrics_t metrics) {
    printf("[Monitor] CPU Usage: %.2f%% | Free Memory: %.2f%%\n",
           metrics.cpu_usage, metrics.mem_free_percent);
}

void log_system_metrics(system_metrics_t metrics) {
    FILE *fp = fopen("logs/metrics.csv", "a");
    if (fp) {
        fprintf(fp, "%.2f,%.2f\n", metrics.cpu_usage, metrics.mem_free_percent);
        fclose(fp);
    }
}

// Only define this function here!
void log_ml_metrics(system_metrics_t metrics, int queue_size, int thread_count, float throughput) {
    FILE *fp = fopen("logs/ml_metrics.csv", "a");
    if (fp) {
        fprintf(fp, "%.2f,%.2f,%d,%d,%.2f\n",
                metrics.cpu_usage, metrics.mem_free_percent, queue_size, thread_count, throughput);
        fclose(fp);
    }
}
