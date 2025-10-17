// a.c
#include <stdio.h>
#define MSG "Hello \
World!\n"
#define _str(x) #x  // #：字符串化操作符
#define _concat(a, b) a##b  // ##：连接操作符
int main() {
  printf(MSG /* "hi!\n" */);
#ifdef __riscv
  printf("Hello RISC-V!\n");
#endif
  _concat(pr, intf)(_str(RISC-V));
  // _concat(pr, intf)=printf
  // _str(RISC-V)="RISC-V"
  return 0;
}
