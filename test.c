#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MAX_USERS 14

char *colors[MAX_USERS] = {"\x1b[31m", "\x1b[32m", "\x1b[33m", "\x1b[34m", "\x1b[35m", "\x1b[36m", "\x1b[37m", "\x1b[41m", "\x1b[42m", "\x1b[43m", "\x1b[44m", "\x1b[45m", "\x1b[46m", "\x1b[47m"} ;
char *usedcolors[MAX_USERS] = {} ;
int colorcount = 0;

char *AssignColor() {
        srand((unsigned)"") ;
        int r ;
        int k = 1 ;
        do {
                r = rand()%MAX_USERS + 1 ;
                if (colorcount < 1) {
                        usedcolors[0] = colors[r] ;
                } else {
                        ++k ;
                }
        } while (strcmp(colors[r], usedcolors[colorcount]) != 0 && (k < colorcount)) ;
        if (colorcount < MAX_USERS) {
                colorcount++ ;
        }
        usedcolors[colorcount] = colors[r] ;
        return colors[r] ;
}


int main() {
        char buffer[256] ;
        memset(buffer, 0, 256) ;
        char name[16] ;
        memset(name, 0, 16) ;
        fgets(name, 16, stdin) ;
        //printf("\x1b[35m%s\x1b[35m", buffer) ;
        sprintf(buffer, "\x1b[0m%s %s\x1b[0m", AssignColor(), name) ;
        printf("%s\n", buffer) ;
        fgets(name, 16, stdin) ;
        sprintf(buffer, "\x1b[0m%s %s\x1b[0m", AssignColor(), name) ;
        printf("%s\n", buffer) ;
        return 0 ;
}
