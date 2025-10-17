# 0 "include.c"
# 0 "<built-in>"
# 0 "<command-line>"
# 1 "/usr/include/stdc-predef.h" 1 3 4
# 0 "<command-line>" 2
# 1 "include.c"
# 1 "./stdio.h" 1







static void my_printf(const char *msg) {

    write(1, msg, strlen(msg));
}
# 2 "include.c" 2

int main() {
    my_printf("Hello from custom stdio!\n");
    return 0;
}
