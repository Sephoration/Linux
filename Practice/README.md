# Linux 应用编程练习

在 Linux 安装 C 编译器和 make 后，进入本目录执行 `make`，`make clean` 清除生成的程序。

- `dir_stats/dir_stats [路径]`：递归统计普通文件数量及逻辑字节数、目录和符号链接。使用 `lstat`，不会跟随符号链接；无权限读取的目录会报错并以非零状态退出。结果不表示磁盘实际占用。
- `pipe_exec/pipe_exec 命令 [参数...] -- 命令 [参数...]`：使用 `pipe`、`fork`、`dup2` 和 `execvp` 连接两个进程，例如 `./pipe_exec/pipe_exec ls -l -- wc -l`。命令不经过 shell，因此不支持重定向、通配符或引号解析；若命令失败返回非零。
- `producer_consumer/producer_consumer`：用互斥锁、条件变量和循环队列实现有界生产者/消费者；观察满队列等待、空队列等待以及结束通知。
