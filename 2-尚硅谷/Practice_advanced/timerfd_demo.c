#define _GNU_SOURCE
#include <errno.h>
#include <inttypes.h>
#include <poll.h>
#include <stdint.h>
#include <time.h>
#include <stdio.h>
#include <sys/timerfd.h>
#include <unistd.h>

/* 通过 poll 等待定时器和标准输入，输入 q 或遇到 EOF 后退出。 */
int main(void) {
    int timer = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC);
    if (timer < 0) { perror("timerfd_create"); return 1; }
    struct itimerspec interval = { .it_interval = {1, 0}, .it_value = {1, 0} };
    if (timerfd_settime(timer, 0, &interval, NULL) < 0) { perror("timerfd_settime"); close(timer); return 1; }
    struct pollfd fds[2] = {{.fd = timer, .events = POLLIN}, {.fd = STDIN_FILENO, .events = POLLIN}};
    int running = 1, result = 0; uint64_t total = 0;
    puts("每秒计时；输入 q 或 Ctrl+D 结束。");
    while (running) {
        if (poll(fds, 2, -1) < 0) { if (errno == EINTR) continue; perror("poll"); result = 1; break; }
        if (fds[0].revents & (POLLERR | POLLHUP | POLLNVAL)) { fputs("定时器出错\n", stderr); result = 1; break; }
        if (fds[0].revents & POLLIN) {
            uint64_t expirations; ssize_t n = read(timer, &expirations, sizeof expirations);
            if (n < 0 && errno == EINTR) continue;
            if (n != (ssize_t)sizeof expirations) { perror("timerfd read"); result = 1; break; }
            total += expirations; printf("经过: %" PRIu64 " 秒\n", total); fflush(stdout);
        }
        if (fds[1].revents & (POLLERR | POLLNVAL)) { fputs("标准输入出错\n", stderr); result = 1; break; }
        if (fds[1].revents & (POLLIN | POLLHUP)) {
            char buffer[128]; ssize_t n = read(STDIN_FILENO, buffer, sizeof buffer);
            if (n < 0 && errno == EINTR) continue;
            if (n < 0) { perror("read"); result = 1; break; }
            if (!n) break;
            for (ssize_t i = 0; i < n; ++i) if (buffer[i] == 'q') running = 0;
        }
    }
    close(timer); return result;
}
