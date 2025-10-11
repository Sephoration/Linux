# 目标：编译、运行并删除 perror_test 测试程序

perror_test : perror_test.c
    -$(CC) -o $@ $^        # 使用 C 编译器编译 perror_test.c，生成可执行文件 perror_test
    -./$@                  # 运行生成的可执行文件 perror_test
    -rm ./$@               # 删除可执行文件，保持目录整洁
