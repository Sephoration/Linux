#define _POSIX_C_SOURCE 200809L
#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

/* 每次发送一行后按发送长度收齐回显，展示 TCP 的字节流边界。 */
int main(int argc, char **argv) {
    if (argc > 2) { fprintf(stderr, "用法: %s [端口]\n", argv[0]); return 2; }
    char *end = NULL; long port = argc == 2 ? strtol(argv[1], &end, 10) : 8080;
    if ((argc == 2 && (!*argv[1] || *end)) || port < 1 || port > 65535) { fputs("无效端口\n", stderr); return 2; }
    signal(SIGPIPE, SIG_IGN);
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) { perror("socket"); return 1; }
    struct sockaddr_in addr = {0}; addr.sin_family = AF_INET;
    addr.sin_port = htons((unsigned short)port); addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (connect(fd, (struct sockaddr *)&addr, sizeof addr) < 0) { perror("connect"); close(fd); return 1; }
    char *line = NULL; size_t capacity = 0; ssize_t length; int result = 0;
    while ((length = getline(&line, &capacity, stdin)) >= 0) {
        size_t sent = 0, received = 0;
        while (sent < (size_t)length) {
            ssize_t n = send(fd, line + sent, (size_t)length - sent, 0);
            if (n > 0) sent += (size_t)n;
            else if (n < 0 && errno == EINTR) continue;
            else { perror("send"); result = 1; goto done; }
        }
        while (received < (size_t)length) {
            char buf[4096]; size_t remain = (size_t)length - received;
            ssize_t n = recv(fd, buf, remain < sizeof buf ? remain : sizeof buf, 0);
            if (n > 0) { if (fwrite(buf, 1, (size_t)n, stdout) != (size_t)n) { perror("stdout"); result = 1; goto done; } received += (size_t)n; }
            else if (n < 0 && errno == EINTR) continue;
            else { if (n < 0) perror("recv"); else fputs("连接已关闭\n", stderr); result = 1; goto done; }
        }
        fflush(stdout);
    }
    if (ferror(stdin)) { perror("stdin"); result = 1; }
done:
    free(line); close(fd); return result;
}
