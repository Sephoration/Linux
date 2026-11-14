#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/inotify.h>
#include <unistd.h>

/* 监听目录中的创建、写入、重命名和删除事件；Ctrl+C 退出。 */
static volatile sig_atomic_t stop;
static void on_signal(int signo) { (void)signo; stop = 1; }
int main(int argc, char **argv) {
    if (argc != 2) { fprintf(stderr, "用法: %s 目录\n", argv[0]); return 2; }
    struct sigaction action = {0}; action.sa_handler = on_signal; sigemptyset(&action.sa_mask);
    if (sigaction(SIGINT, &action, NULL) < 0 || sigaction(SIGTERM, &action, NULL) < 0) { perror("sigaction"); return 1; }
    int fd = inotify_init1(IN_CLOEXEC | IN_NONBLOCK);
    if (fd < 0) { perror("inotify_init1"); return 1; }
    int wd = inotify_add_watch(fd, argv[1], IN_CREATE | IN_DELETE | IN_MODIFY | IN_MOVED_FROM | IN_MOVED_TO | IN_DELETE_SELF | IN_MOVE_SELF | IN_ONLYDIR);
    if (wd < 0) { perror("inotify_add_watch"); close(fd); return 1; }
    printf("监听 %s，按 Ctrl+C 结束\n", argv[1]);
    int failed = 0;
    while (!stop) {
        struct pollfd watched = { .fd = fd, .events = POLLIN };
        int n = poll(&watched, 1, 500);
        if (n < 0) { if (errno == EINTR) continue; perror("poll"); failed = 1; break; }
        if (watched.revents & (POLLERR | POLLHUP | POLLNVAL)) { fprintf(stderr, "监听描述符异常\n"); failed = 1; break; }
        if (!(watched.revents & POLLIN)) continue;
        union { char bytes[8192]; struct inotify_event alignment; } buffer;
        ssize_t count = read(fd, buffer.bytes, sizeof buffer.bytes);
        if (count < 0) { if (errno == EINTR || errno == EAGAIN) continue; perror("read"); failed = 1; break; }
        for (size_t offset = 0; offset < (size_t)count;) {
            const struct inotify_event *event = (const struct inotify_event *)(buffer.bytes + offset);
            if (event->mask & IN_Q_OVERFLOW) fprintf(stderr, "事件队列溢出，部分事件丢失\n");
            else {
                printf("mask=0x%08x cookie=%u %s\n", event->mask, event->cookie, event->len ? event->name : "(目录本身)");
                if (event->mask & IN_IGNORED) stop = 1;
            }
            offset += sizeof *event + event->len;
        }
        fflush(stdout);
    }
    if (close(fd) < 0) { perror("close"); failed = 1; }
    return failed;
}
