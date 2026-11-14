#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

/* 信号处理器只设置 sig_atomic_t；主循环负责输出和资源清理。 */
static volatile sig_atomic_t stop = 0;
static volatile sig_atomic_t reports = 0;
static void handle_signal(int signal_number) {
    if (signal_number == SIGINT || signal_number == SIGTERM) stop = 1;
    else if (signal_number == SIGUSR1) reports = 1;
}
int main(void) {
    struct sigaction action = {0}; action.sa_handler = handle_signal; sigemptyset(&action.sa_mask);
    if (sigaction(SIGINT, &action, NULL) || sigaction(SIGTERM, &action, NULL) || sigaction(SIGUSR1, &action, NULL)) { perror("sigaction"); return 1; }
    printf("PID: %ld；发送 SIGUSR1 查看运行时间，Ctrl+C 退出\n", (long)getpid()); fflush(stdout);
    struct timespec started, now;
    if (clock_gettime(CLOCK_MONOTONIC, &started)) { perror("clock_gettime"); return 1; }
    while (!stop) {
        struct timespec delay = {0, 200000000};
        while (nanosleep(&delay, &delay) < 0 && errno == EINTR && !stop) {}
        if (reports) {
            reports = 0;
            if (clock_gettime(CLOCK_MONOTONIC, &now)) { perror("clock_gettime"); return 1; }
            printf("运行 %.1f 秒\n", (now.tv_sec - started.tv_sec) + (now.tv_nsec - started.tv_nsec) / 1000000000.0); fflush(stdout);
        }
    }
    puts("退出"); return 0;
}
