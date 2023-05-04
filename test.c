#include <stdio.h>

int main() {
        char buffer[256] ;
        fgets(buffer, 256, stdin) ;
        printf("\x1b[35m%s\x1b[35m", buffer) ;
        return 0 ;
}
