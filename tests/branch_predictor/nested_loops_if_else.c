#include <stdio.h>

int main() {
    int loops = 150;
    int loops2 = 30;
    int s_even = 0;
    int s_odd = 0;
    int p;

    int i = 1;
    while (i <= loops) {       
        int j = 1;
        while (j <= loops2) {
            if (j % 2 == 0) {
                s_even += j;    
            } else {
                s_odd += j;
            }
            j++;
        }
        i++;        
    }

    p = s_even * s_odd;

    printf("%x %x %x\n", s_even, s_odd, p);

    return 0;
}
