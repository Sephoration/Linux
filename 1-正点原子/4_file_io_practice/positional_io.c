#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

/* pread 不改变文件偏移量：从指定偏移读取一段并显示十六进制。 */
int main(int argc, char **argv) {
    if (argc != 4) { fprintf(stderr, "用法: %s 文件 偏移 字节数(1-256)\n", argv[0]); return 2; }
    char *end = NULL;
    errno = 0; intmax_t offset = strtoimax(argv[2], &end, 10);
    if (errno || end == argv[2] || *end || offset < 0 || (intmax_t)(off_t)offset != offset) { fprintf(stderr, "无效偏移\n"); return 2; }
    errno = 0; long count = strtol(argv[3], &end, 10);
    if (errno || end == argv[3] || *end || count < 1 || count > 256) { fprintf(stderr, "无效字节数\n"); return 2; }
    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) { perror("open"); return 1; }
    unsigned char buf[256]; ssize_t n;
    do { n = pread(fd, buf, (size_t)count, (off_t)offset); } while (n < 0 && errno == EINTR);
    if (n < 0) { perror("pread"); close(fd); return 1; }
    for (ssize_t i = 0; i < n; ++i) printf("%02x%s", (unsigned)buf[i], (i + 1) % 16 == 0 || i + 1 == n ? "\n" : " ");
    if (!n) puts("已到文件末尾");
    if (close(fd) < 0) { perror("close"); return 1; }
    return 0;
}
