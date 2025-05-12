#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "http_server.h"

float g_cpu = 0.0, g_mem = 0.0, g_throughput = 0.0;
int g_queue = 0, g_threads = 0;

void *http_server_thread(void *arg) {
    int server_fd, client_fd;
    struct sockaddr_in addr;
    char buffer[4096];

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(8080);

    bind(server_fd, (struct sockaddr *)&addr, sizeof(addr));
    listen(server_fd, 5);

    while (1) {
        client_fd = accept(server_fd, NULL, NULL);
        int n = read(client_fd, buffer, sizeof(buffer)-1);
        buffer[n] = 0;

        if (strstr(buffer, "GET /metrics")) {
            char json[256];
            snprintf(json, sizeof(json),
                "{\"cpu\":%.2f,\"mem\":%.2f,\"queue\":%d,\"threads\":%d,\"throughput\":%.2f}\n",
                g_cpu, g_mem, g_queue, g_threads, g_throughput);

            dprintf(client_fd,
                "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n%s", json);
        } else {
            // Serve dashboard.html
            FILE *f = fopen("dashboard.html", "r");
            if (f) {
                dprintf(client_fd, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n");
                char line[512];
                while (fgets(line, sizeof(line), f)) {
                    write(client_fd, line, strlen(line));
                }
                fclose(f);
            } else {
                dprintf(client_fd, "HTTP/1.1 404 Not Found\r\n\r\n404 Not Found");
            }
        }
        close(client_fd);
    }
    close(server_fd);
    return NULL;
}
