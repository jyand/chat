#include "config.h"

int main(int argc, char **argv) {
        if (argc < 2) {
                error("Please specify a hostname.") ;
        }
        int sockfd = socket(AF_INET, SOCK_STREAM, 0) ;
        if (sockfd < 0) {
                error("Error! Couldn't open socket") ;
        }
        struct hostent *server = gethostbyname(argv[1]) ;
        if (server == NULL) {
                error("Error! Host does not exist.") ;
        }
        struct sockaddr_in serv_addr ;
        socklen_t slen = sizeof(serv_addr) ;
        memset((char*)&serv_addr, 0, sizeof(serv_addr)) ;
        serv_addr.sin_family = AF_INET ;
        memcpy((char*)server->h_addr, (char*)&serv_addr.sin_addr.s_addr, server->h_length) ;
        serv_addr.sin_port = htons(PORT_NUM) ;
        int status = connect(sockfd, (struct sockaddr*)&serv_addr, slen) ;
        if (status < 0) {
                error("Error! Couldn't connect") ;
        }
        char buffer[256] ;
        int n ;
        for (;;) {
                printf("enter a message: ") ;
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
                memset(buffer, 0, 256) ;
                n = recv(sockfd, buffer, 255, 0) ;
                if (n < 0) {
                        error("Error! Couldn't read from socket") ;
                }
                printf("response: %s\n", buffer) ;
        }
}
