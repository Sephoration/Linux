#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

/* 两个命令之间建立匿名管道，命令通过 PATH 查找；不经过 shell。 */
static int wait_child(pid_t pid) {
    int status;
    while (waitpid(pid, &status, 0) < 0) { if (errno != EINTR) { perror("waitpid"); return 1; } }
    if (WIFEXITED(status)) return WEXITSTATUS(status);
    return WIFSIGNALED(status) ? 128 + WTERMSIG(status) : 1;
}
int main(int argc, char **argv) {
    int sep = 1;
    while (sep < argc && strcmp(argv[sep], "--")) ++sep;
    if (sep <= 1 || sep >= argc - 1) { fprintf(stderr, "用法: %s 命令 [参数...] -- 命令 [参数...]\n", argv[0]); return 2; }
    argv[sep] = NULL;
    int fd[2]; if (pipe(fd) < 0) { perror("pipe"); return 1; }
    pid_t left = fork();
    if (left < 0) { perror("fork"); close(fd[0]); close(fd[1]); return 1; }
    if (left == 0) {
        if (dup2(fd[1], STDOUT_FILENO) < 0) { perror("dup2"); _exit(126); }
        close(fd[0]); close(fd[1]); execvp(argv[1], &argv[1]); perror(argv[1]); _exit(127);
    }
    pid_t right = fork();
    if (right < 0) { perror("fork"); close(fd[0]); close(fd[1]); wait_child(left); return 1; }
    if (right == 0) {
        if (dup2(fd[0], STDIN_FILENO) < 0) { perror("dup2"); _exit(126); }
        close(fd[0]); close(fd[1]); execvp(argv[sep + 1], &argv[sep + 1]); perror(argv[sep + 1]); _exit(127);
    }
    close(fd[0]); close(fd[1]);
    int a = wait_child(left), b = wait_child(right);
    return b ? b : a;
}
