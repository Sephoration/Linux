#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

/* fork 多个子进程并回收，观察退出状态和被信号终止的情况。 */
int main(void) {
    int started = 0, failed = 0;
    for (int i = 0; i < 3; ++i) {
        pid_t pid = fork();
        if (pid < 0) { perror("fork"); failed = 1; break; }
        if (pid == 0) {
            printf("子进程 %ld，父进程 %ld，任务 %d\n", (long)getpid(), (long)getppid(), i);
            fflush(stdout);
            _exit(i + 1);
        }
        ++started;
    }
    for (int i = 0; i < started; ++i) {
        int status; pid_t pid;
        do { pid = waitpid(-1, &status, 0); } while (pid < 0 && errno == EINTR);
        if (pid < 0) { perror("waitpid"); failed = 1; break; }
        if (WIFEXITED(status)) printf("子进程 %ld 退出码 %d\n", (long)pid, WEXITSTATUS(status));
        else if (WIFSIGNALED(status)) printf("子进程 %ld 被信号 %d 终止\n", (long)pid, WTERMSIG(status));
    }
    return failed;
}
