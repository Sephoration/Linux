#define _XOPEN_SOURCE 700
#include <errno.h>
#include <ftw.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

/* 使用 FTW_PHYS 不跟随符号链接；不可读目录也会计入错误。 */
static unsigned long files, directories, links, errors;
static int visit(const char *path, const struct stat *st, int type, struct FTW *info) {
    (void)info;
    switch (type) {
        case FTW_F: if (S_ISREG(st->st_mode)) ++files; break;
        case FTW_D: ++directories; break;
        case FTW_SL: case FTW_SLN: ++links; break;
        case FTW_DNR: case FTW_NS: fprintf(stderr, "无法读取: %s\n", path); ++errors; return 0;
        default: break;
    }
    puts(path);
    return 0;
}
int main(int argc, char **argv) {
    if (argc > 2) { fprintf(stderr, "用法: %s [路径]\n", argv[0]); return 2; }
    const char *path = argc == 2 ? argv[1] : ".";
    if (nftw(path, visit, 16, FTW_PHYS) < 0) { fprintf(stderr, "nftw: %s\n", strerror(errno)); return 1; }
    fprintf(stderr, "普通文件: %lu，目录: %lu，链接: %lu，错误: %lu\n", files, directories, links, errors);
    return errors ? 1 : 0;
}
