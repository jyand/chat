#include "config.h"

struct UserNode *head = NULL ;
struct UserNode *tail = NULL ;

void AddTail(int newclisockfd, char *newusername, char *newcolor) {
        if (head == NULL) {
                head = (struct UserNode*)malloc(sizeof(struct UserNode)) ;
                head->clisockfd = newclisockfd ;
                head->username = (char*)malloc(strlen(newusername)*sizeof(char)) ;
                strcpy(head->username, newusername) ;
                head->color = (char*)malloc(strlen(newcolor)*sizeof(char)) ;
                strcpy(head->color, newcolor) ;
                head->next = NULL ;
                tail = head ;
        } else {
                tail->next = (struct UserNode*)malloc(sizeof(struct UserNode)) ;
                tail->next->clisockfd = newclisockfd ;
                tail->next->username = (char*)malloc(strlen(newusername)*sizeof(char)) ;
                strcpy(tail->next->username, newusername) ;
                tail->next->color = (char*)malloc(strlen(newcolor)*sizeof(char)) ;
                strcpy(tail->next->color, newcolor) ;
                tail->next->next = NULL ;
                tail = tail->next ;
        }
}

void Broadcast(int fromfd, char *message) {
        struct sockaddr_in cliaddr ;
        socklen_t clen = sizeof(cliaddr) ;
        if (getpeername(fromfd, (struct sockaddr*)&cliaddr, &clen) < 0) {
                error("Error! Unkown sender") ;
        }
        struct UserNode *current = head ;
        while (current != NULL) {
                if (current->clisockfd != fromfd) {
                        char buffer[512] ;
                        sprintf(buffer, "[%s]:%s", inet_ntoa(cliaddr.sin_addr), message) ;
                        int nmsg = strlen(buffer) ;
                        int nsend = send(current->clisockfd, buffer, nmsg, 0) ;
                        if (nsend != nmsg) {
                                error("Error! send() failed") ;
                        }
                }
                current = current->next ;
        }
}

void *ServerThread(void *args) {
        pthread_detach(pthread_self()) ;
        int clisockfd = ((struct ThreadArgs*)args)->clisockfd ;
        free(args) ;
        char buffer[256] ;
        int nrcv = recv(clisockfd, buffer, 256, 0) ;
        if (nrcv < 0) {
                error("Error! recv() failed") ;
        }
        struct UserNode current = head ;
        struct UserNode prev = head ;
        int tempfd ;
        while (current != NULL) {
                tempfd = current->clisockfd ;
                if (tempfd == clisockfd) {
                        break ;
                }
                if (current != head) {
                        prev = prev->next ;
                }
                current = current->next ;
        }
        if (nrcv == 0) {
                printf("%d disconnected\n", current->username) ;
                if (current == head) {
                        head = current->next ;
                        free(current) ;
                } else {
                        prev->next = current->next ;
                        free(current) ;
                }
                close(clisockfd) ;
                return NULL ;
        } else {
                Broadcast(clisockfd, buffer) ;
        }
        while (nrcv > 0) {
                /*int nsend = send(clisockfd, buffer, nrcv, 0) ;
                if (nsend != nrcv) {
                        error("Error! send() failed") ;
                }*/
                memset(buffer, 0, sizeof(buffer)) ;
                nrcv = recv(clisockfd, buffer, 256, 0) ;
                if (nrcv < 0) {
                        error("Error! recv() failed") ;
                } else {
                        Broadcast(clisockfd, buffer) ;
                }
        }
        if (current == head) {
                head = current->next ; 
        } else {
                prev->next = current->next ;
                free(current) ;
        }
        close(clisockfd) ;
        return NULL ;
}

int main(int argc, char **argv) {
        int sockfd = socket(AF_INET, SOCK_STREAM, 0) ;
        if (sockfd < 0) {
                error("Error! Coudn't open socket.") ;
        }
        struct sockaddr_in serv_addr ;
        socklen_t slen = sizeof(serv_addr) ;
        memset((char*)&serv_addr, 0, sizeof(serv_addr)) ;
        serv_addr.sin_family = AF_INET ;
        serv_addr.sin_addr.s_addr = INADDR_ANY ;
        serv_addr.sin_port = htons(PORT_NUM) ;
        int status = bind(sockfd, (struct sockaddr*)&serv_addr, slen) ;
        if (status < 0) {
                error("Error! bind() failed") ;
        }
        listen(sockfd, 5) ;
        for (;;) {
                struct sockaddr_in cli_addr ;
                socklen_t clen = sizeof(cli_addr) ;
                int newsockfd = accept(sockfd, (struct sockaddr*)&cli_addr, &clen) ;
                if (newsockfd < 0) {
                        error("Error! accept() failed") ;
                }
                printf("Connected: %s\n", inet_ntoa(cli_addr.sin_addr)) ;
                AddTail(newsockfd) ;
                struct ThreadArgs *args = (struct ThreadArgs*)malloc(sizeof(struct ThreadArgs)) ;
                if (args == NULL) {
                        error("Error! Couldn't create thread argument.") ;
                }
                args->clisockfd = newsockfd ;
                pthread_t tid ;
                if (pthread_create(&tid, NULL, ServerThread, (void*)args) != 0) {
                        error("Error! Couldn't create a new thread.") ;
                }
        }
        return 0 ;
}
