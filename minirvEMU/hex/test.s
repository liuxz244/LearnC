# 用lui加载基础地址 (假设地址在低内存)
# 以0x1000作为测试内存起始
_start:
    lui     x1, 0x1         # x1 = 0x1000 （测试基址）
    
    # 测试 addi 正数
    addi    x2, x0, 123     # x2 = 123
    # 测试 addi 负数
    addi    x3, x0, -45     # x3 = -45 (0xFFD3)
    
    # 用sw存储word: 验证正数
    sw      x2, 0(x1)       # *(0x1000) = 123
    
    # 用sw存储word: 验证负数
    sw      x3, 4(x1)       # *(0x1004) = -45
    
    # 用lw读回验证正数
    lw      x4, 0(x1)       # x4应为123
    # 用lw读回验证负数
    lw      x5, 4(x1)       # x5应为-45
    
    # 测试lbu（零扩展），先存一个byte < 0x80
    addi    x6, x0, 0x7F    # 127
    sb      x6, 8(x1)       # *(0x1008) = 0x7F
    lbu     x7, 8(x1)       # x7应为127
    
    # 测试lbu 对高位bit的零扩展
    addi    x6, x0, -1      # 立即数 -1 => 0xFFFFFFFF
    sb      x6, 9(x1)       # 存低8位 0xFF
    lbu     x8, 9(x1)       # x8应为0x000000FF(255)
    
    # 测试 add 组合正负
    add     x9, x4, x5      # 123 + (-45) = 78
    
    # 测试跳转 jalr，调用子函数并返回
    addi    x10, x0, 4      # 传参寄存器，测试值
    addi    x11, x0, 0      # 存返回值
    addi    x12, x0, func   # func标签地址加载（需用lui+addi组合，见下）
    jalr    x1, x12, 0      # 调用函数，返回到 x1里存的地址
    
    # 程序结束（死循环）
end_loop:
    addi    x10, x0, 0
    ebreak
    jalr    x0, x0, end_loop

func:
    # 简单操作：输入寄存器x10，加常数返回
    addi    x11, x10, 7      # 返回值 = 参数 + 7
    jalr    x0, x1, 0        # 从x1返回
