#include "config.h"

char *colors[MAX_USERS] = {"\x1b[31m", "\x1b[32m", "\x1b[33m", "\x1b[34m", "\x1b[35m", "\x1b[36m", "\x1b[37m", "\x1b[41m", "\x1b[42m", "\x1b[43m", "\x1b[44m", "\x1b[45m", "\x1b[46m", "\x1b[47m"};
char *usedcolors[MAX_USERS] = {} ;
int colorcount = 0;

struct UserNode *user_head = NULL;
struct UserNode *user_tail = NULL;

struct ChatRoom {
    char *room_code;
    struct UserNode *user_head;
    struct UserNode *user_tail;
    struct ChatRoom *next;
};

struct ChatRoom *chatroom_head = NULL;
struct ChatRoom *chatroom_tail = NULL;

// Function to create a new chat room and add it to the list
struct ChatRoom *create_chatroom(const char *room_code) {
    struct ChatRoom *new_room = (struct ChatRoom *)malloc(sizeof(struct ChatRoom));
    new_room->room_code = strdup(room_code);
    new_room->user_head = NULL;
    new_room->user_tail = NULL;
    new_room->next = NULL;

    if (chatroom_tail == NULL) {
        chatroom_head = chatroom_tail = new_room;
    } else {
        chatroom_tail->next = new_room;
        chatroom_tail = new_room;
    }

    return new_room;
}

// Function to find a chat room by its room code
struct ChatRoom *find_chatroom_by_code(const char *room_code) {
    struct ChatRoom *current = chatroom_head;
    while (current != NULL) {
        if (strcmp(current->room_code, room_code) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

// Function to add a user to a chat room
void add_user_to_chatroom(struct ChatRoom *chatroom, struct UserNode *user) {
    if (chatroom->user_tail == NULL) {
        chatroom->user_head = chatroom->user_tail = user;
    } else {
        chatroom->user_tail->next = user;
        chatroom->user_tail = user;
    }
}

void ShowConnected() {
    struct UserNode *current = user_head;
    if (user_head == NULL) {
        printf("empty\n");
    } else {
        while (current != NULL) {
            printf("%s\n", current->username);
            current = current->next;
        }
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

void AddTail(int newclisockfd, char *newusername, char *newcolor) {
    if (user_head == NULL) {
        user_head = (struct UserNode *)malloc(sizeof(struct UserNode));
        user_head->clisockfd = newclisockfd;
        user_head->username = (char *)malloc(strlen(newusername) * sizeof(char));
        strcpy(user_head->username, newusername);
        user_head->color = (char *)malloc(strlen(newcolor) * sizeof(char));
        strcpy(user_head->color, newcolor);
        user_head->next = NULL;
        user_tail = user_head;
    } else {
        user_tail->next = (struct UserNode *)malloc(sizeof(struct UserNode));
        user_tail->next->clisockfd = newclisockfd;
        user_tail->next->username = (char *)malloc(strlen(newusername) * sizeof(char));
        strcpy(user_tail->next->username, newusername);
        user_tail->next->color = (char *)malloc(strlen(newcolor) * sizeof(char));
        strcpy(user_tail->next->color, newcolor);
        user_tail->next->next = NULL;
        user_tail = user_tail->next;
    }
}

void Broadcast(int fromfd, char *message) {
        struct sockaddr_in cliaddr;
        socklen_t clen = sizeof(cliaddr);
        if (getpeername(fromfd, (struct sockaddr *)&cliaddr, &clen) < 0) {
                error("Error! Unknown sender");
        }
        char *username;
        char *color;
        struct UserNode *current = user_head;
        while (current != NULL) {
                if (current->clisockfd == fromfd) {
                        username = current->username;
                        color = current->color;
                        break;
                }
                current = current->next;
        }
        current = user_head;
        while (current != NULL) {
                if (current->clisockfd != fromfd) {
                        char buffer[512];
                        sprintf(buffer, "\x1b[0m%s[%s@%s]: %s\x1b[0m", color, username, inet_ntoa(cliaddr.sin_addr), message);
                        int nmsg = strlen(buffer);
                        int nsend = send(current->clisockfd, buffer, nmsg, 0);
                        if (nsend != nmsg) {
                                error("Error! send() failed");
                        }
                }
                current = current->next;
        }
}

void *ServerThread(void *args) {
        pthread_detach(pthread_self());
        int clisockfd = ((struct ThreadArgs *)args)->clisockfd;
        struct ChatRoom *chatroom = ((struct ThreadArgs *)args)->chatroom;
        free(args);
        char buffer[256];
        int nrcv = recv(clisockfd, buffer, 255, 0);
        struct UserNode *current = user_head;
        struct UserNode *prev = user_head;
        while (current != NULL) {
                int temp = current->clisockfd;
                if (temp == clisockfd) {
                        break;
                }
                if (current != user_head) {
                        prev = prev->next;
                }
                current = current->next;
        }
        if (nrcv < 0) {
                error("Error! recv() failed");
        }
        if (nrcv == 0) {
                printf("%s disconnected\n", current->username);
                if (current == user_head) {
                        user_head = current->next;
                        free(current);
                } else {
                        prev->next = current->next;
                        free(current);
                }
                close(clisockfd);
                return NULL;
        } else {
                Broadcast(clisockfd, buffer);
        }
        while (nrcv != 0) {
                if (nrcv < 0) {
                        error("Error! recv() failed");
                } else {
                        Broadcast(clisockfd, buffer);
                }
                memset(buffer, 0, sizeof(buffer));
                nrcv = recv(clisockfd, buffer, 255, 0);
        }
        printf("%s disconnected\n", current->username);
        if (current == user_head) {
                user_head = current->next;
                free(current);
        } else {
                prev->next = current->next;
                free(current);
        }
        close(clisockfd);
        return NULL;
}

int main(int argc, char *argv[]) {
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

        while (1) {
                struct sockaddr_in cliaddr;
                socklen_t clen = sizeof(cliaddr);
                int clisockfd = accept(sockfd, (struct sockaddr *)&cliaddr, &clen);
                if (clisockfd < 0) {
                    error("Error! accept() failed");
                }

                char username[256];
                int nrcv = recv(clisockfd, username, 255, 0);
                if (nrcv < 0) {
                    error("Error! recv() failed");
                }
                username[nrcv] = '\0';

                printf("%s connected\n", username);

                char *color = AssignColor();
                AddTail(clisockfd, username, color);

                struct ThreadArgs *args = (struct ThreadArgs *)malloc(sizeof(struct ThreadArgs));
                args->clisockfd = clisockfd;
                args->chatroom = NULL;  // No chat room for now

                pthread_t thread_id;
                if (pthread_create(&thread_id, NULL, ServerThread, (void *)args) != 0) {
                    error("Error! pthread_create() failed");
                }
            }

        close(sockfd);
        return 0;
}
