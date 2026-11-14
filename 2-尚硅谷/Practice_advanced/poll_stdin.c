#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <poll.h>
#include <stdio.h>
#include <unistd.h>

/* 同时等待标准输入和超时，不依靠不停查询；Ctrl+D 结束。 */
int main(void) {
    struct pollfd input = { .fd = STDIN_FILENO, .events = POLLIN };
    char buffer[1024];
    for (;;) {
        int result = poll(&input, 1, 3000);
        if (result < 0) { if (errno == EINTR) continue; perror("poll"); return 1; }
        if (!result) { fputs("等待输入...\n", stderr); continue; }
        if (input.revents & (POLLERR | POLLNVAL)) { fputs("输入描述符错误\n", stderr); return 1; }
        if (input.revents & (POLLIN | POLLHUP)) {
            ssize_t n = read(STDIN_FILENO, buffer, sizeof buffer);
            if (n < 0) { if (errno == EINTR) continue; perror("read"); return 1; }
            if (!n) break;
            size_t offset = 0;
            while (offset < (size_t)n) {
                ssize_t written = write(STDOUT_FILENO, buffer + offset, (size_t)n - offset);
                if (written > 0) offset += (size_t)written;
                else if (written < 0 && errno == EINTR) continue;
                else { perror("write"); return 1; }
            }
        }
    }
    return 0;
}
