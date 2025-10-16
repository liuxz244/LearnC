# 找到当前目录下的所有 .c 文件，去掉后缀得到可执行文件名列表
TARGETS = $(basename $(wildcard *.c))

CFLAGS=-Wall -g

all:
	$(TARGETS) 
clean:
	rm -f $(TARGETS) 
