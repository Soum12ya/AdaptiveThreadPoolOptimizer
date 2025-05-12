#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

extern float g_cpu, g_mem, g_throughput;
extern int g_queue, g_threads;

void *http_server_thread(void *arg);

#endif
