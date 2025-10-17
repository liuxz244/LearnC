#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

/* sISA
 7  6 5  4 3   2 1   0
+----+----+-----+-----+
| 00 | rd | rs1 | rs2 | R[rd]=R[rs1]+R[rs2]       add指令, 寄存器相加
+----+----+-----+-----+
| 01 |  保 留   | rs2 | out rs2                   out指令, 将R[rs2]输出到终端
+----+----+-----+-----+
| 10 | rd |    imm    | R[rd]=imm                 li指令, 装入立即数, 高位补0
+----+----+-----+-----+
| 11 |   addr   | rs2 | if (R[0]!=R[rs2]) PC=addr bner0指令, 若不等于R[0]则跳转
+----+----------+-----+
*/
#define LI(rd, imm)        (0x80 | ((rd)<<4) | (imm))
#define ADD(rd, rs1, rs2)  (0x00 | ((rd)<<4) | ((rs1)<<2) | (rs2))
#define BNER0(addr, rs2)   (0xC0 | ((addr & 0x1F)<<2) | (rs2))
#define OUT(rs)            (0x40 | (rs))

uint8_t PC = 0;  // PC位宽为4位
uint8_t R[4];    // GPR有4个, 位宽均为8位
uint8_t M[16] = {
    LI(1, 0),
    LI(2, 1),
    ADD(1, 1, 2),
    LI(3, 1),
    ADD(2, 2, 3),
    BNER0(2, 2),
    OUT(1)
};
// 最多只有16条指令，内存大小为16字节即可

int quit = 0;

void inst_cycle() {
    uint8_t inst = M[PC];  // 取出当前指令
    //printf("PC=%02X, Inst=%02X, ", PC, M[PC]);

    uint8_t opcode = (inst & 0xC0) >> 6;  // inst[7:6]
    uint8_t rd     = (inst & 0x30) >> 4;  // inst[5:4]
    uint8_t rs1    = (inst & 0x0C) >> 2;  // inst[3:2]
    uint8_t rs2    = (inst & 0x03)     ;  // inst[1:0]
    uint8_t imm    = (inst & 0x0F)     ;  // inst[3:0]
    uint8_t addr   = (inst & 0x3C) >> 2;  // inst[5:2]

    switch(opcode) {
        case 0: // 00: add
            R[rd] = R[rs1] + R[rs2];
            //printf("R[%d] = R[%d] + R[%d] -> %d\n", rd, rs1, rs2, R[rd]);
            break;
        case 1: // 01: out
            printf("R[%d]=%d\n", rs2, R[rs2]);
            quit = 1;
            break;
        case 2: // 10: li
            R[rd] = imm;
            //printf("R[%d] = %d\n", rd, R[rd]);
            break;
        case 3: // 11: bner0
            //printf("R[%d]:%02X, ", rs2, R[rs2]);
            if (R[rs2] != R[0]) {
                PC = addr - 1;  // 跳转到addr处，-1是因为后面会+1
                //printf("跳转到%02X\n", PC + 1);
            } else {
                //printf("不跳转\n");
            }
            break;
        default:
            //printf("NOP\n");
            break;
    }

    PC = (PC + 1) % 16;    // 更新PC
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "没有指定数列的末项\n");
        return 1;
    }
    R[0] = atoi(argv[1]) + 1;  // 如果数列末项过大，R[1]会溢出

    while (!quit) {
        inst_cycle();
    }
moxiang
    return 0;
}