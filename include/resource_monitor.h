#ifndef RESOURCE_MONITOR_H
#define RESOURCE_MONITOR_H

typedef struct {
    float cpu_usage;
    float mem_free_percent;
} system_metrics_t;

system_metrics_t get_system_metrics();
void print_system_metrics(system_metrics_t metrics);
void log_system_metrics(system_metrics_t metrics);
void log_ml_metrics(system_metrics_t metrics, int queue_size, int thread_count, float throughput);

#endif
