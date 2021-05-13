#include <stdio.h>

int main() {
    int loops1 = 150;
    int loops2 = 21;
    int s = 0;
    int s1 = 0;
    int p;

    for (int i = 1 ; i <= loops1 ; i++) {
        s += i;
        for (int j = 1 ; j <= loops2 ; j++) {
            s1 += j;
        }
    }

    p = s * s1;

    printf("%x %x %x\n", s, s1, p);

    return 0;
}
