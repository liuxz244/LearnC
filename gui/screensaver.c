#include <am.h>
#include <klib-macros.h>

#define WIDTH  400
#define HEIGHT 300

static uint32_t buf[WIDTH * HEIGHT];

void draw(uint32_t color) {
    // 填充缓冲区为指定颜色
    for (int i = 0; i < WIDTH * HEIGHT; i++) {
        buf[i] = color;
    }

    // 将缓冲区画到屏幕
    io_write(AM_GPU_FBDRAW, 0, 0, buf, WIDTH, HEIGHT, true);
}

int main() {
    ioe_init(); // 初始化 GUI

    while (1) {
        draw(0x000000ff);
    }
    
    return 0;
}