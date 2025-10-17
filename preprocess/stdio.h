// include/stdio.h
#ifndef MY_STDIO_H
#define MY_STDIO_H

// 模拟一个简单的输出
#define printf(...) my_printf(__VA_ARGS__)

static void my_printf(const char *msg) {
    // 本质上只是演示，不是真正的完整 printf
    write(1, msg, strlen(msg));
}

#endif
