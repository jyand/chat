#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT_NUM 1004
#define MAX_ROOMS 5
#define MAX_USERS 20

struct ThreadArgs {
        int clisockfd ;
} ;

void error(const char *msg) {
        perror(msg) ;
        exit(1) ;
}
