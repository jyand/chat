void parse_command(char *input_string, char **argv) {
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
}

void *ClientSendThread(void *args) {
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
}
