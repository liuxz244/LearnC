# 找到当前目录下的所有 .c 文件，去掉后缀得到可执行文件名列表
TARGETS = $(basename $(wildcard *.c))

CFLAGS=-Wall -g

ex19: object.o
ex22_main: ex22.o
clean:
	rm -f $(TARGETS) *.o
