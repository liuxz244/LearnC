#include <am.h>
#include <klib-macros.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#define WIDTH   400
#define HEIGHT  300

// 目标颜色
static const uint32_t COLORS[] = {
    0x000000, 0xff0000,
    0x00ff00, 0x0000ff,
    0xffff00, 0xff00ff,
    0x00ffff, 0xffffff
};
#define NUM_COLORS (sizeof(COLORS) / sizeof(COLORS[0]))

// 显示缓冲区
static uint32_t framebuffer[WIDTH * HEIGHT];

// 颜色分量拆解与合成
static inline uint8_t R(uint32_t c) { return (c >> 16) & 0xff; }
static inline uint8_t G(uint32_t c) { return (c >> 8) & 0xff; }
static inline uint8_t B(uint32_t c) { return c & 0xff; }
static inline uint32_t pixel(uint8_t r, uint8_t g, uint8_t b) {
    return (r << 16) | (g << 8) | b;
}

// 绘制整屏颜色
static void draw_screen(uint32_t color) {
    for (int i = 0; i < WIDTH * HEIGHT; i++) {
        framebuffer[i] = color;
    }
    io_write(AM_GPU_FBDRAW, 0, 0, framebuffer, WIDTH, HEIGHT, true);
}


int main(void) {
    ioe_init();  // 初始化 GUI

    // 当前颜色与目标颜色索引
    int current_idx = 0;
    int target_idx  = 1;
    uint32_t current_color = COLORS[current_idx];
    uint32_t target_color  = COLORS[target_idx];

    // 渐变参数
    const int steps          = 30; // 渐变步数
    const int delay_normal   = 30; // 正常速度(ms)
    const int delay_fast     = 10; // 加速速度(ms)
    int       step_delay     = delay_normal;

    int step_counter        = 0;    // 当前渐变的步数
    unsigned long last_time = 0;
    bool speed_up           = false;

    // FPS 统计
    unsigned long fps_last_time = 0;
    int fps_count = 0;

    while (1) {
        
        // 处理键盘事件
        while (1) {
            AM_INPUT_KEYBRD_T ev = io_read(AM_INPUT_KEYBRD);
            if (ev.keycode == AM_KEY_NONE) break;

            if (ev.keydown) {
                if (ev.keycode == AM_KEY_ESCAPE) {
                    printf("ESC pressed, exiting...\n");
                    return 0;
                }
                speed_up = true;
            } else {
                if (ev.keycode != AM_KEY_ESCAPE) {
                    speed_up = false;
                }
            }
        }

        // 根据按键状态选择速度
        step_delay = speed_up ? delay_fast : delay_normal;

        unsigned long now = io_read(AM_TIMER_UPTIME).us / 1000;

        // 控制逐步颜色变化
        if (now - last_time >= (unsigned long)step_delay) {
            float t = (float)step_counter / (float)steps; // 渐变比例 (0~1)

            uint8_t R0 = R(current_color), G0 = G(current_color), B0 = B(current_color);
            uint8_t R1 = R(target_color),  G1 = G(target_color),  B1 = B(target_color);

            uint8_t Ri = (uint8_t)(R0 + (R1 - R0) * t);
            uint8_t Gi = (uint8_t)(G0 + (G1 - G0) * t);
            uint8_t Bi = (uint8_t)(B0 + (B1 - B0) * t);

            draw_screen(pixel(Ri, Gi, Bi)); // 绘制当前颜色
            fps_count++;

            step_counter++;
            last_time = now;

            // 渐变完成，切换到下一个目标颜色
            if (step_counter > steps) {
                current_color = target_color;
                current_idx   = target_idx;
                step_counter  = 0;
                target_idx    = (target_idx + 1) % NUM_COLORS;
                target_color  = COLORS[target_idx];
            }
        }

        // 每秒输出一次 FPS
        if (now - fps_last_time >= 1000) {
            printf("FPS = %d\n", fps_count);
            fps_last_time = now;
            fps_count = 0;
        }
    }

    return 0;
    
}