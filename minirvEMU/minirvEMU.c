#include <klib-macros.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <am.h>

#define WRITE_REG(idx, value) do {if(idx!=0) R[idx] = (value);} while(0)  // 确保R[0]始终为0
#define MEM_SIZE (1 * 1024 * 1024)  // 模拟 1MB 内存
uint32_t PC = 0;
uint32_t lastPC = 0;
uint32_t R[32];
uint8_t *M = NULL;
int same_cnt = 0;
int ebreak = 0;

#define VIDEO_MEM_BASE 0x20000000
#define VIDEO_MEM_SIZE (256 * 256 * 4)    // 256 KB
#define VIDEO_MEM_LIMIT (VIDEO_MEM_BASE + VIDEO_MEM_SIZE)

// 视频显存
uint8_t video_mem[VIDEO_MEM_SIZE];  // 存放像素数据 (RGBA 每像素 4字节)

static inline int is_video_addr(uint32_t addr) {
    return addr >= VIDEO_MEM_BASE && addr < VIDEO_MEM_LIMIT;
}

// 读一个字节
static inline uint8_t mem_read8(uint32_t addr) {
    if (is_video_addr(addr)) {
        return video_mem[addr - VIDEO_MEM_BASE];
    } else {
        return M[addr];
    }
}
// 写一个字节
static inline void mem_write8(uint32_t addr, uint8_t val) {
    if (is_video_addr(addr)) {
        video_mem[addr - VIDEO_MEM_BASE] = val;
    } else {
        M[addr] = val;
    }
}
// 读一个 32 位字
static inline uint32_t mem_read32(uint32_t addr) {
    if (is_video_addr(addr)) {
        addr -= VIDEO_MEM_BASE;
        return (uint32_t)video_mem[addr] |
               ((uint32_t)video_mem[addr+1] << 8) |
               ((uint32_t)video_mem[addr+2] << 16) |
               ((uint32_t)video_mem[addr+3] << 24);
    } else {
        return (uint32_t)M[addr] |
               ((uint32_t)M[addr+1] << 8) |
               ((uint32_t)M[addr+2] << 16) |
               ((uint32_t)M[addr+3] << 24);
    }
}
// 写一个 32 位字
static inline void mem_write32(uint32_t addr, uint32_t val) {
    if (is_video_addr(addr)) {
        addr -= VIDEO_MEM_BASE;
        video_mem[addr]   =  val        & 0xFF;
        video_mem[addr+1] = (val >> 8)  & 0xFF;
        video_mem[addr+2] = (val >> 16) & 0xFF;
        video_mem[addr+3] = (val >> 24) & 0xFF;
    } else {
        M[addr]   =  val        & 0xFF;
        M[addr+1] = (val >> 8)  & 0xFF;
        M[addr+2] = (val >> 16) & 0xFF;
        M[addr+3] = (val >> 24) & 0xFF;
    }
}

/**
 * 从指定的 hex 文件读取数据到内存
 * @param filename 文件路径
 * @return 成功返回分配好的内存指针，失败返回 NULL
 */
uint8_t* init_memory_from_file(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if(!fp) {
        perror("无法打开文件");
        return NULL;
    }

    M = malloc(MEM_SIZE);
    if(!M) {
        fprintf(stderr, "内存分配失败\n");
        fclose(fp);
        return NULL;
    }
    memset(M, 0, MEM_SIZE);  // 初始化为 0

    char line[64];
    int idx = 0;
    while(fgets(line, sizeof(line), fp)) {
        if(line[0] == '\n' || line[0] == '\r') continue;
        uint32_t value = (uint32_t)strtoul(line, NULL, 16);
        mem_write32(idx, value);  // 小端存储：低字节在低地址
        idx += 4;
    }
    fclose(fp);

    /*
    // 测试内存加载
    for (int addr = 0; addr < MEM_SIZE; addr += 4) {
        uint32_t word = mem_read32(addr);
        printf("M[%d] = 0x%08X\n", addr, word);
    }
    */
    return M;
}

/**
 * 检测是否出现死循环
 * 连续两次以上 PC 未变化，则认为死循环
 * @return: 1 表示检测到死循环，0 表示正常
 */
int detect_deadloop() {
    if(PC == lastPC) {
        if(++same_cnt >= 2) {
            fprintf(stderr, "检测到死循环，程序终止\n");
            return 1;
        }
    } else {
        same_cnt = 0;
        lastPC = PC;
    }
    return 0;
}

/**
 * 对低位的立即数进行符号扩展
 * @param val   原始值（无符号数），仅低 bits 位有效
 * @param bits  原始数据的位宽（包括符号位），取值范围 1~32
 * @return 经过符号扩展后的 32 位有符号整数。
 */
int32_t sign_extend(uint32_t val, int bits) {
    int32_t s = (int32_t)(val << (32 - bits)) >> (32 - bits);
    return s;
}

void draw_video_buffer() {
    int width = 256;
    int height = 256;

    // 直接把视频显存的每一行送给 AM_GPU_FBDRAW
    for (int y = 0; y < height; y++) {
        io_write(AM_GPU_FBDRAW, 0, y, 
                &video_mem[y * width * 4], 
                width, 1, false);
    }
    // 最后刷新一次显示
    io_write(AM_GPU_FBDRAW, 0, 0, NULL, 0, 0, true);
}


void inst_cycle() {
    uint32_t inst = mem_read32(PC);  // 取出当前指令
    printf("PC=%04X:  ", PC);

    uint32_t opcode = (inst & 0x0000007F)      ;  // inst[6:0]
    uint32_t rd     = (inst & 0x00000F80) >> 7 ;  // inst[11:7]
    uint32_t funct3 = (inst & 0x00007000) >> 12;  // inst[14:12]
    uint32_t rs1    = (inst & 0x000F8000) >> 15;  // inst[19:15]
    uint32_t rs2    = (inst & 0x01F00000) >> 20;  // inst[24:20]
    uint32_t funct7 = (inst & 0xFE000000) >> 25;  // inst[31:25]
    uint32_t imm_u  = (inst & 0xFFFFF000)      ;  // inst[31:12]
    int32_t  imm_i  = sign_extend((inst >> 20) & 0xFFF, 12);  // inst[31:20]
    int32_t  imm_s  = sign_extend(((inst >> 25) & 0x7F) << 5 | ((inst >> 7) & 0x1F), 12);// inst[31:25|11:7]
    
    switch(opcode) {
        case 0x03:  // 0000011
            // lw
            if(funct3 == 0x2) {
                uint32_t addr = R[rs1] + imm_i;
                WRITE_REG(rd, mem_read32(addr));
                printf("R[%d] = M[%u] -> %d\n", rd, addr, R[rd]);
            } else if(funct3 == 0x4) {
            // lbu
                uint32_t addr = R[rs1] + imm_i;
                WRITE_REG(rd, (uint32_t)mem_read8(addr));
                printf("R[%d] = M[%u] -> %d\n", rd, addr, R[rd]);
            }
            PC += 4;
            break;
        case 0x13:  // 0010011
            // addi
            if(funct3 == 0x0) {
                WRITE_REG(rd, R[rs1] + imm_i);
                printf("R[%d] = R[%d] + %d -> %d\n", rd, rs1, imm_i, R[rd]);
            }
            PC += 4;
            break;
        case 0x23:  // 0100011
            // sw
            if(funct3 == 0x2) {
                uint32_t addr = R[rs1] + imm_s;
                mem_write32(addr, R[rs2]);
                printf("M[%u] = R[%d] -> %d\n", addr, rs2, R[rs2]);
            } else if(funct3 == 0x0) {
            // sb
                uint32_t addr = R[rs1] + imm_s;
                mem_write8(addr, R[rs2] & 0xFF);
                printf("M[%u] = R[%d][7:0] -> %d\n", addr, rs2, mem_read8(addr));
            }
            PC += 4;
            break;
        case 0x33:  // 0110011
            // add
            if(funct3 == 0x0 && funct7 == 0x00) {
                WRITE_REG(rd, R[rs1] + R[rs2]);
                printf("R[%d] = R[%d] + R[%d] -> %d\n", rd, rs1, rs2, R[rd]);
            }
            PC += 4;
            break;
        case 0x37:  // 0110111 
            // lui
            WRITE_REG(rd, imm_u);
            printf("R[%d] = %d\n", rd, imm_u);
            PC += 4;
            break;
        case 0x67:  // 1100111
            // jalr
            uint32_t target = R[rs1] + imm_i;
            WRITE_REG(rd, PC + 4);
            PC = target & ~1;  // 注意rs1 = rd的情况，先用rs1计算目标地址，再写回rd
            printf("JALR to %X, R[%d] = %d\n", PC, rd, R[rd]);
            break;
        case 0x73:  // 1110011
            // ebreak 
            if (funct3 == 0x0 && imm_i == 1) {
                if (R[10] == 0) { 
                    printf("HIT GOOD TRAP\n");
                } else {
                    printf("HIT BAD TRAP\n");
                }
                ebreak = 1;  // 设置停止标志
            }
            PC += 4;
            break;
        default:
            printf("未知指令: 0x%08X at PC=%X\n", inst, PC);
            break;
    }
}


int main(void) {
    ioe_init();  // 初始化 GUI

    M = init_memory_from_file("hex/vga.hex");
    if(!M) { fprintf(stderr, "内存分配失败\n"); return 1; }

    int cycles = 0;
    while(cycles < 628000 && !ebreak) {
        inst_cycle();  // 执行一条指令
        if(detect_deadloop()) break;
        cycles++;
    }
    //printf("R[10] = %d\n", R[10]);  // 输出 a0 的值
    draw_video_buffer();   // 程序执行结束后绘制显存到屏幕

    while(1);  // 保持GUI显示
    free(M);
    return 0;
}