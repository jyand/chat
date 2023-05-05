#include "config.h"

/*void parse_command(char *input_string, char **argv) {
    int i = 0;
    char *token;

    // initialize argv to NULL
    for (i = 0; i < MAX_BUFFER; i++) {
        argv[i] = NULL;
    }

    // tokenize input_string and store tokens in argv
    i = 0;
    token = strtok(input_string, " ");
    while (token != NULL) {
        argv[i++] = token;
        token = strtok(NULL, " ");
    }
    argv[i] = NULL;
}*/

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

/*void *ClientSendThread(void *args) {
    pthread_detach(pthread_self());
    int sockfd = ((struct ThreadArgs*)args)->clisockfd;
    free(args);
    char input_buffer[256];
    char *argv[5];
    int n;

    for (;;) {
        // read input from console and tokenize it
        fgets(input_buffer, 255, stdin);
        parse_command(input_buffer, argv);

        // send the tokens to the server
        for (int i = 0; i < 5 && argv[i] != NULL; ++i) {
            n = send(sockfd, argv[i], strlen(argv[i]), 0);
            if (n < 0) {
                error("Error! Couldn't write to socket");
            }
        }
    }

    return NULL;
}*/

void *ClientSendThread(void *args) {
        pthread_detach(pthread_self()) ;
        int sockfd = ((struct ThreadArgs*)args)->clisockfd ;
        free(args) ;
        char buffer[256] ;
        int n ;
        bool validname = false ;
        do {
                if (validname == false) {
                        printf("make a username\n") ;
                } else {
                        printf("\nme: ") ;
                }
                memset(buffer, 0, 256) ;
                fgets(buffer, 255, stdin) ;
                if (strlen(buffer) == 1) {
                        buffer[0] = '\0' ;
                }
                if (buffer[strlen(buffer)-1] == NEWLINE_VALUE) {
                        buffer[strlen(buffer)-1] = '\0' ;
                }
                if (validname == false && strlen(buffer) > 12) {
                        continue ;
                } else {
                        validname = true ;
                }
                n = send(sockfd, buffer, strlen(buffer), 0) ;
                if (n < 0) {
                        error("Error! Couldn't write to socket") ;
                }
        } while (n != 0) ;
        return NULL ;
}

int main(int argc, char **argv) {
        if (argc < 2) {
                error("Please specify a hostname.") ;
        }
        int sockfd = socket(AF_INET, SOCK_STREAM, 0) ;
        if (sockfd < 0) {
                error("Error! Couldn't open socket") ;
        }
        struct sockaddr_in serv_addr ;
        socklen_t slen = sizeof(serv_addr) ;
        memset((char*)&serv_addr, 0, sizeof(serv_addr)) ;
        serv_addr.sin_family = AF_INET ;
        serv_addr.sin_addr.s_addr = inet_addr(argv[1]) ;
        serv_addr.sin_port = htons(PORT_NUM) ;
        //printf("Try connecting to %s... \n", inet_ntoa(serv_addr.sin_addr)) ;
        int status = connect(sockfd, (struct sockaddr*)&serv_addr, slen) ;
        if (status < 0) {
                error("Error! Couldn't connect") ;
        }
        // multi-room part
        bool newroom = false ; 
        char buffer[256] ;
        memset(buffer, 0, 256) ;
        if (argc > 2) {
                strcpy(buffer, argv[2]) ;
        }
        if (strcmp(buffer, "new") == 0) {
                newroom = true ;
        } else {
                int roomnum = atoi(buffer) ;
                if (roomnum < 1 && roomnum > 5) {
                        roomnum = 1 ;
                }
                memset(buffer, 0, 256) ;
                int nrcv = recv(sockfd, buffer, 255, 0) ;
                if (nrcv < 0) {
                        error("Error! recv() failed") ;
                }
                printf("\n%d", buffer) ;
        }
        if (newroom == true) {
                memset(buffer, 0, 256) ;
                int nrcv = recv(sockfd, buffer, 255, 0) ;
                if (nrcv < 0) {
                        error("Error! recv() failed") ;
                }
                printf("\n%s\n", buffer) ;
        }
        //pthread_t tid[2] ;
        pthread_t tidtx ;
        pthread_t tidrx ;
        struct ThreadArgs *args = (struct ThreadArgs*)malloc(sizeof(struct ThreadArgs)) ;
        args->clisockfd = sockfd ;
        pthread_create(&tidtx, NULL, ClientSendThread, (void*)args) ;
        args = (struct ThreadArgs*)malloc(sizeof(struct ThreadArgs)) ;
        args->clisockfd = sockfd ;
        pthread_create(&tidrx, NULL, ClientRecvThread, (void*)args) ;
        pthread_join(tidtx, NULL) ;
        close(sockfd) ;
        return 0 ;
}
