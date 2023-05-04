#include <arpa/inet.h>
#include <math.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT_NUM 1030
#define MAX_ROOMS 5
#define MAX_USERS 14
#define NEWLINE_VALUE 0xA

struct ThreadArgs {
        int clisockfd ;
} ;

struct UserNode {
        int clisockfd ;
        char *username ;
        char *color ;
        struct UserNode *next ;
} ;

void error(const char *msg) {
        perror(msg) ;
        exit(1) ;
}
