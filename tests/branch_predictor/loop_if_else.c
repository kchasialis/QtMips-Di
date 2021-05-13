#include <stdio.h>

int main() {
    int loops = 150;
    int s_even = 0;
    int s_odd = 0;
    int p;

    int i = 1;
    while (i <= loops) {
        if (i % 2 == 0) {
            s_even += i;
        } else {
            s_odd += i;
        } 
        i++;        
    }

    p = s_even * s_odd;

    printf("%x %x %x\n", s_even, s_odd, p);

    return 0;
}
