#include "config.h"

void *ClientRecvThread(void *args) {
        pthread_detach(pthread_self()) ;
        int sockfd = ((struct ThreadArgs*)args)->clisockfd ;
        free(args) ;
        char buffer[512] ;
        int n = recv(sockfd, buffer, 512, 0) ;
        while (n > 0) {
                memset(buffer, 0, 512) ;
                n = recv(sockfd, buffer, 512, 0) ;
                if (n < 0) {
                        error("Error! recv() failed") ;
                }
                printf("\n%s\n", buffer) ;
        }
        return NULL ;
}

void *ClientSendThread(void *args) {
        pthread_detach(pthread_self()) ;
        int sockfd = ((struct ThreadArgs*)args)->clisockfd ;
        free(args) ;
        char buffer[256] ;
        int n ;
        for (;;) {
                memset(buffer, 0, 256) ;
                fgets(buffer, 255, stdin) ;
                if (strlen(buffer) == 1) {
                        buffer[0] = '\0' ;
                }
                n = send(sockfd, buffer, strlen(buffer), 0) ;
                if (n < 0) {
                        error("Error! Couldn't write to socket") ;
                }
                if (n == 0) {
                        break ;
                }
        }
        return NULL ;
}

int main(int argc, char **argv) {
        if (argc < 2) {
                error("Please specify a hostname.") ;
        }
        if (argc < 3) {
                error("Please choose a username.") ;
        }
        char *username = argv[3] ;
        int sockfd = socket(AF_INET, SOCK_STREAM, 0) ;
        if (sockfd < 0) {
                error("Error! Couldn't open socket") ;
        }
        struct sockaddr_in serv_addr ;
        socklen_t slen = sizeof(serv_addr) ;
        memset((char*)&serv_addr, 0, sizeof(serv_addr)) ;
        serv_addr.sin_family = AF_INET ;
        serv_addr.sin_port = htons(PORT_NUM) ;
        printf("Try connecting to %s... \n", inet_ntoa(serv_addr.sin_addr)) ;
        int status = connect(sockfd, (struct sockaddr*)&serv_addr, slen) ;
        if (status < 0) {
                error("Error! Couldn't connect") ;
        }
        pthread_t tid[2] ;
        struct ThreadArgs *args = (struct ThreadArgs*)malloc(sizeof(struct ThreadArgs)) ;
        args->clisockfd = sockfd ;
        pthread_create(&tid[0], NULL, ClientSendThread, (void*)args) ;
        args = (struct ThreadArgs*)malloc(sizeof(struct ThreadArgs)) ;
        args->clisockfd = sockfd ;
        pthread_create(&tid[1], NULL, ClientRecvThread, (void*)args) ;
        pthread_join(tid[0], NULL) ;
        close(sockfd) ;
        return 0 ;
}
