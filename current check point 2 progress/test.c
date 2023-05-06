#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MAX_USERS 14

char *colors[MAX_USERS] = {"\x1b[31m", "\x1b[32m", "\x1b[33m", "\x1b[34m", "\x1b[35m", "\x1b[36m", "\x1b[37m", "\x1b[41m", "\x1b[42m", "\x1b[43m", "\x1b[44m", "\x1b[45m", "\x1b[46m", "\x1b[47m"} ;

// not unique for now
char *AssignColor() {
        srand((unsigned)"") ;
        int k = rand()%MAX_USERS ;
        return colors[k] ;
}


int main() {
        char buffer[256] ;
        memset(buffer, 0, 256) ;
        char name[16] ;
        fgets(name, 16, stdin) ;
        //printf("\x1b[35m%s\x1b[35m", buffer) ;
        sprintf(buffer, "\x1b[0m%s %s\x1b[0m", AssignColor(), name) ;
        printf("%s\n", buffer) ;
        return 0 ;
}
