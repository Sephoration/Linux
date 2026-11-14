#define _POSIX_C_SOURCE 200809L
#include <fcntl.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

/* 只读映射统计字节和行；运行时不要同时截断输入文件。 */
int main(int argc, char **argv) {
    if (argc != 2) { fprintf(stderr, "用法: %s 文件\n", argv[0]); return 2; }
    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) { perror("open"); return 1; }
    struct stat st;
    if (fstat(fd, &st) < 0) { perror("fstat"); close(fd); return 1; }
    if (!S_ISREG(st.st_mode) || st.st_size < 0 || (uintmax_t)st.st_size > SIZE_MAX) {
        fprintf(stderr, "需要可映射的普通文件\n"); close(fd); return 1;
    }
    size_t size = (size_t)st.st_size;
    if (!size) { puts("字节: 0\n行: 0"); close(fd); return 0; }
    const unsigned char *data = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (data == MAP_FAILED) { perror("mmap"); close(fd); return 1; }
    size_t lines = 0;
    for (size_t i = 0; i < size; ++i) if (data[i] == '\n') ++lines;
    if (data[size - 1] != '\n') ++lines;
    printf("字节: %zu\n行: %zu\n", size, lines);
    int failed = 0;
    if (munmap((void *)data, size) < 0) { perror("munmap"); failed = 1; }
    if (close(fd) < 0) { perror("close"); failed = 1; }
    return failed;
}
