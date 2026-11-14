#define _POSIX_C_SOURCE 200809L
#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

/* 单连接回显服务。仅绑定本机；Ctrl+C 中断 accept/recv 并退出。 */
static volatile sig_atomic_t stopping = 0;
static void on_signal(int sig) { (void)sig; stopping = 1; }
static int send_all(int fd, const char *data, size_t length) {
    size_t sent = 0;
    while (sent < length && !stopping) {
        ssize_t n = send(fd, data + sent, length - sent, 0);
        if (n > 0) sent += (size_t)n;
        else if (n < 0 && errno == EINTR) continue;
        else return -1;
    }
    return sent == length ? 0 : -1;
}
int main(int argc, char **argv) {
    char *end = NULL;
    long port = 8080;
    if (argc > 2) { fprintf(stderr, "用法: %s [端口]\n", argv[0]); return 2; }
    if (argc == 2) { port = strtol(argv[1], &end, 10); if (!*argv[1] || *end || port < 1 || port > 65535) { fputs("无效端口\n", stderr); return 2; } }
    struct sigaction sa = {0}; sa.sa_handler = on_signal; sigemptyset(&sa.sa_mask);
    if (sigaction(SIGINT, &sa, NULL) < 0 || sigaction(SIGTERM, &sa, NULL) < 0) { perror("sigaction"); return 1; }
    signal(SIGPIPE, SIG_IGN);
    int server = socket(AF_INET, SOCK_STREAM, 0);
    if (server < 0) { perror("socket"); return 1; }
    int enabled = 1;
    if (setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &enabled, sizeof enabled) < 0) { perror("setsockopt"); close(server); return 1; }
    struct sockaddr_in address = {0}; address.sin_family = AF_INET;
    address.sin_port = htons((unsigned short)port); address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (bind(server, (struct sockaddr *)&address, sizeof address) < 0 || listen(server, 8) < 0) { perror("bind/listen"); close(server); return 1; }
    printf("监听 127.0.0.1:%ld\n", port); fflush(stdout);
    while (!stopping) {
        struct pollfd ready = { .fd = server, .events = POLLIN };
        int available = poll(&ready, 1, 500);
        if (available == 0 || (available < 0 && errno == EINTR)) continue;
        if (available < 0) { perror("poll"); break; }
        int client = accept(server, NULL, NULL);
        if (client < 0) { if (errno == EINTR) continue; perror("accept"); break; }
        struct timeval timeout = { .tv_sec = 1, .tv_usec = 0 };
        if (setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout) < 0 ||
            setsockopt(client, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof timeout) < 0) {
            perror("setsockopt"); close(client); continue;
        }
        char buf[4096];
        while (!stopping) {
            ssize_t n = recv(client, buf, sizeof buf, 0);
            if (n == 0) break;
            if (n < 0) { if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK) continue; perror("recv"); break; }
            if (send_all(client, buf, (size_t)n) < 0) { if (!stopping) perror("send"); break; }
        }
        close(client);
    }
    close(server); return 0;
}
