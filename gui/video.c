#include <amtest.h>

#define FPS 30
#define N   32  // // 将屏幕划分为 N×N 的方格

static inline uint32_t pixel(uint8_t r, uint8_t g, uint8_t b) {
  return (r << 16) | (g << 8) | b;
}
static inline uint8_t R(uint32_t p) { return p >> 16; }
static inline uint8_t G(uint32_t p) { return p >> 8; }
static inline uint8_t B(uint32_t p) { return p; }

// 用来存储每个方格的颜色值
static uint32_t canvas[N][N];
// 用来标记每个方格是否已更新过颜色
static int used[N][N];

// 颜色缓冲区，用来一次性绘制一个方块的所有像素
static uint32_t color_buf[32 * 32];

// 将 canvas 中的方格颜色绘制到屏幕上
void redraw() {
  // 每个方格的像素宽高（屏幕的每个大方格大小）
  int w = io_read(AM_GPU_CONFIG).width / N;
  int h = io_read(AM_GPU_CONFIG).height / N;
  // 每个方格总像素数量
  int block_size = w * h;
  assert((uint32_t)block_size <= LENGTH(color_buf));

  int x, y, k;
  for (y = 0; y < N; y ++) {
    for (x = 0; x < N; x ++) {
      // 为当前方块填充颜色
      for (k = 0; k < block_size; k ++) {
        color_buf[k] = canvas[y][x];  // 当前方块全填相同颜色
      }
      // 绘制方块
      io_write(AM_GPU_FBDRAW, x * w, y * h, color_buf, w, h, false);
    }
  }
  // 通知 GPU 将所有改动刷新到屏幕
  io_write(AM_GPU_FBDRAW, 0, 0, NULL, 0, 0, true);
}

// 根据时间计数生成一种颜色
static uint32_t p(int tsc) {
  int b = tsc & 0xff;  // 取 tsc 的低 8 位
  return pixel(b * 6, b * 7, b);
}

// 更新 canvas 数组中的颜色值
void update() {
  static int tsc = 0;
  static int dx[4] = {0, 1, 0, -1};  // 四个方向的 x 增量（下右上左）
  static int dy[4] = {1, 0, -1, 0};  // 四个方向的 y 增量（下右上左）

  tsc ++;

  // 初始化 used 数组，标记所有方格未访问
  for (int i = 0; i < N; i ++)
    for (int j = 0; j < N; j ++) {
      used[i][j] = 0;
    }

  // 设置起始方格 (0,0) 的颜色
  int init = tsc * 1;
  canvas[0][0] = p(init); used[0][0] = 1;
  // 定义当前位置和方向
  int x = 0, y = 0, d = 0;  // 从左上角开始，初始方向向下
  // 按螺旋方式填充整个方格的颜色
  for (int step = 1; step < N * N; step ++) {
    for (int t = 0; t < 4; t ++) {
      int x1 = x + dx[d], y1 = y + dy[d];  // 计算下一个位置
      // 如果下一个位置合法且未访问
      if (x1 >= 0 && x1 < N && y1 >= 0 && y1 < N && !used[x1][y1]) {
        x = x1; y = y1;
        used[x][y] = 1;
        // 根据 step 填充颜色，慢慢变换颜色
        canvas[x][y] = p(init + step / 2);
        break;
      }
      // 改变方向（顺时针切换）
      d = (d + 1) % 4;
    }
  }
}

// 视频测试主循环
void video_test() {
  unsigned long last = 0;      // 上一帧时间戳（毫秒）
  unsigned long fps_last = 0;  // 上一次打印 FPS 的时间戳
  int fps = 0;

  while (1) {
    // 获取当前系统已运行毫秒数
    unsigned long upt = io_read(AM_TIMER_UPTIME).us / 1000;
    // 控制刷新频率，达到预设 FPS
    if (upt - last > 1000 / FPS) {
      update(); // 更新 canvas 颜色数据
      redraw(); // 绘制到屏幕
      last = upt;
      fps ++;
    }
    // 每隔 1 秒打印一次当前 FPS
    if (upt - fps_last > 1000) {
      // display fps every 1s
      printf("%d: FPS = %d\n", upt, fps);
      fps_last = upt;
      fps = 0;  // 重置下一秒的帧数计数
    }
  }
}
