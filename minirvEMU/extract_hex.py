#!/usr/bin/env python3
import sys

if len(sys.argv) != 3:
    print(f"用法: {sys.argv[0]} 输入文件 输出文件")
    sys.exit(1)

infile = sys.argv[1]
outfile = sys.argv[2]

with open(infile, 'r', encoding='utf-8') as fin, open(outfile, 'w', encoding='utf-8') as fout:
    for line_num, line in enumerate(fin):
        # 跳过第一行
        if line_num == 0:
            continue

        line = line.strip()
        if not line:
            continue
        # 如果有冒号，去掉地址部分
        if ':' in line:
            _, data = line.split(':', 1)
        else:
            data = line
        # 按空格分割每个十六进制数
        hex_words = data.strip().split()
        # 分行写入
        for word in hex_words:
            fout.write(word + '\n')
