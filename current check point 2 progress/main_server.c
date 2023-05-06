#include "config.h"

char *colors[MAX_USERS] = {"\x1b[31m", "\x1b[32m", "\x1b[33m", "\x1b[34m", "\x1b[35m", "\x1b[36m", "\x1b[37m", "\x1b[41m", "\x1b[42m", "\x1b[43m", "\x1b[44m", "\x1b[45m", "\x1b[46m", "\x1b[47m"};
char *usedcolors[MAX_USERS] = {};
int colorcount = 0;

#define MAX_CHAT_ROOMS 10

struct ChatRoom {
    struct UserNode *head;
    struct UserNode *tail;
} chatRooms[MAX_CHAT_ROOMS];

struct ChatRoom *findChatRoom(int chatRoomIndex);
int chatRoomCount = 0;

// Find a chat room by its index
struct ChatRoom *findChatRoom(int chatRoomIndex) {
    if (chatRoomIndex >= 0 && chatRoomIndex < MAX_CHAT_ROOMS) {
        return &chatRooms[chatRoomIndex];
    }
    return NULL;
}

void ShowConnected(struct UserNode *head, int chatRoomIndex);
void AddTail(struct UserNode **head, struct UserNode **tail, int newclisockfd, char *newusername, char *newcolor);
void Broadcast(struct UserNode *head, int chatRoomIndex, int fromfd, char *message);
void *ServerThread(void *args);

// Add a new chat room and return its index
int addChatRoom() {
    if (chatRoomCount < MAX_CHAT_ROOMS) {
        struct ChatRoom *newChatRoom = &chatRooms[chatRoomCount];
        newChatRoom->head = NULL;
        newChatRoom->tail = NULL;
        chatRoomCount++;
        return chatRoomCount - 1;
    }
    return -1;
}

void AddTail(struct UserNode **head, struct UserNode **tail, int newclisockfd, char *newusername, char *newcolor) {
    if (*head == NULL) {
        *head = (struct UserNode*)malloc(sizeof(struct UserNode));
        (*head)->clisockfd = newclisockfd;
        (*head)->username = (char*)malloc(strlen(newusername) * sizeof(char));
        strcpy((*head)->username, newusername);
        (*head)->color = (char*)malloc(strlen(newcolor) * sizeof(char));
        strcpy((*head)->color, newcolor);
        (*head)->next = NULL;
        *tail = *head;
    } else {
        (*tail)->next = (struct UserNode*)malloc(sizeof(struct UserNode));
        (*tail)->next->clisockfd = newclisockfd;
        (*tail)->next->username = (char*)malloc(strlen(newusername) * sizeof(char));
        strcpy((*tail)->next->username, newusername);
        (*tail)->next->color = (char*)malloc(strlen(newcolor) * sizeof(char));
        strcpy((*tail)->next->color, newcolor);
        (*tail)->next->next = NULL;
        *tail = (*tail)->next;
    }
}

char *AssignColor() {
        int r ;
        int k = 0 ;
        do {
                r = rand()%MAX_USERS ;
                if (colorcount < 1) {
                        usedcolors[0] = colors[r] ;
                } else {
                        ++k ;
                }
        } while (strcmp(colors[r], usedcolors[k]) == 0 && (k < colorcount)) ;
        colorcount++ ;
        usedcolors[colorcount] = colors[r] ;
        return colors[r] ;
}

void ShowConnected(struct UserNode *head, int chatRoomIndex) {
    struct ChatRoom *chatRoom = &chatRooms[chatRoomIndex];
        if (chatRoom == NULL) {
            return;
        }
    
        struct UserNode *current = head ;
        if (head == NULL) {
                printf("empty\n") ;
        } else {
                while (current != NULL) {
                        printf("%s\n", current->username) ;
                        current = current->next ;
                }
        }
}

void Broadcast(struct UserNode *head, int chatRoomIndex, int fromfd, char *message) {
        struct ChatRoom *chatRoom = &chatRooms[chatRoomIndex];
        if (chatRoom == NULL) {
            return;
        }
    
        struct sockaddr_in cliaddr ;
        socklen_t clen = sizeof(cliaddr) ;
        if (getpeername(fromfd, (struct sockaddr*)&cliaddr, &clen) < 0) {
                error("Error! Unkown sender") ;
        }
        char *username ;
        char *color ;
        struct UserNode *current = head ;
        while (current != NULL) {
                if (current->clisockfd == fromfd) {
                        username = current->username ;
                        color = current->color ;
                        break ;
                }
                current = current->next ;
        }
        current = head ;
        while (current != NULL) {
                if (current->clisockfd != fromfd) {
                        char buffer[512] ;
                        sprintf(buffer, "\x1b[0m%s[%s@%s]: %s\x1b[0m", color, username, inet_ntoa(cliaddr.sin_addr), message) ;
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
    int clisockfd = ((struct ThreadArgs *)args)->clisockfd;
        int chatRoomIndex = ((struct ThreadArgs *)args)->chatRoomIndex;
        struct UserNode *head = chatRooms[chatRoomIndex].head;
        pthread_detach(pthread_self());
        free(args);
        char buffer[256] ;
        int nrcv = recv(clisockfd, buffer, 255, 0) ;
        struct UserNode *current = head ;
        struct UserNode *prev = head ;
        while (current != NULL) {
                int temp = current->clisockfd ;
                if (temp == clisockfd) {
                        break ;
                }
                if (current != head) {
                        prev = prev->next ;
                }
                current = current->next ;
        }
        if (nrcv < 0) {
                error("Error! recv() failed") ;
        }
        if (nrcv == 0) {
                printf("%s disconnected\n", current->username) ;
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
            Broadcast(head, chatRoomIndex, clisockfd, buffer);
        }
        while (nrcv != 0) {
                if (nrcv < 0) {
                        error("Error! recv() failed") ;
                } else {
                    Broadcast(head, chatRoomIndex, clisockfd, buffer);
                }
                memset(buffer, 0, sizeof(buffer)) ;
                nrcv = recv(clisockfd, buffer, 255, 0) ;
        }
        printf("%s disconnected\n", current->username) ;
        if (current == head) {
                head = current->next ;
                free(current) ;
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
                char namebuffer[256] ;
                memset(namebuffer, 0, 256) ;
                int nrcv = recv(newsockfd, namebuffer, 255, 0) ;
                if (nrcv < 0) {
                        error("Error! recv() failed") ;
                }
                printf("Connected: %s@%s\n", namebuffer, inet_ntoa(cli_addr.sin_addr)) ;
                AddTail(&chatRooms[chatRoomIndex].head, &chatRooms[chatRoomIndex].tail, newsockfd, namebuffer, AssignColor());
                ShowConnected(chatRooms[chatRoomIndex].head, chatRoomIndex);
            
                int chatRoomIndex = 0; // Logic to determine chat room index goes here
                AddTail(&chatRooms[chatRoomIndex].head, &chatRooms[chatRoomIndex].tail, newsockfd, namebuffer, AssignColor());

                ShowConnected(chatRooms[chatRoomIndex].head, chatRoomIndex);

                struct ThreadArgs *args = (struct ThreadArgs *)malloc(sizeof(struct ThreadArgs));
                args->clisockfd = newsockfd;
                args->chatRoomIndex = chatRoomIndex; // Add this line

                pthread_t tid;
                if (pthread_create(&tid, NULL, ServerThread, (void *)args) != 0) {
                        error("Error! Couldn't create a new thread.");
                }
        }
    
        return 0;
}
