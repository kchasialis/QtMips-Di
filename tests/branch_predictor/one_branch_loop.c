#include <stdio.h>

int main() {
    int loops = 150;
    int s = 0;
    int s1 = 0;
    int p;

    for (int i = 1 ; i <= loops ; i++) {
        s += i;
        s1 += i * 2;
    }

    p = s1 * s;

    printf("%x %x %x\n", s, s1, p);

    return 0;
}
