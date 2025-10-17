#include <stdio.h>

int main() {

    long sum = 0;
    int i = 1;
    do {
        sum = sum + i;
        i = i + 1;
    } while (i <= 1000000000);

    printf("sum = %ld\n", sum);
    return 0;

}
