#include "myutils.h"
#define QUEUE_SIZE 20


int open_listener(char *portStr, int queueSize){
    struct addrinfo hint, *potentialAddrList, *potentialAddr;
    int status, sock;

    memset(&hint, 0, sizeof(struct addrinfo));
    hint.ai_family   = AF_UNSPEC;
    hint.ai_socktype = SOCK_STREAM;
    hint.ai_flags    = AI_PASSIVE;

    status = getaddrinfo(NULL, portStr, &hint, &potentialAddrList);
    if (status != 0) {
        err_die(gai_strerror(status));
    }

    // attempt to create socket
    for (potentialAddr = potentialAddrList; potentialAddr != NULL; potentialAddr = potentialAddr->ai_next){
        sock = socket(potentialAddr->ai_family, potentialAddr->ai_socktype, potentialAddr->ai_protocol);

        if (sock == -1) continue;
        
        //bind successful socket; now bind
        status = bind(sock, potentialAddr->ai_addr, potentialAddr->ai_addrlen);
        if (status != 0) {
            close(sock);
            continue;
        }

        //enable listening for incoming connection requests
        status = listen(sock, queueSize);
        if (status != 0) {
            close(sock);
            continue;
        }

        //good socket found
        break;
    }

    freeaddrinfo(potentialAddrList);

    return sock;

}


//------------------MAIN------------------

int main(int argc, char *argv[]){
    int queueSock;
    tttstep *step;
    tttgame *game;
    pthread_mutex_t *myMutex = malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(myMutex, NULL);


    //create listener for connections
    queueSock = open_listener(argv[1], QUEUE_SIZE);
    if (queueSock == -1){
        err_die("open_listener failed");
    }

    printf("Listening on port %s\n", argv[1]);
    fflush(stdout);

    //accept connections
    while (1){
        game = create_tttgame();

        //pairing, waiting, begin, etc...
        {
            //accept playerX connection
            *(game->playerXSocket) = accept(queueSock, NULL, NULL);
            if (*(game->playerXSocket) == -1){
                printf("accept failed, try a different port.\n");
                fflush(stdout);
                break;
            }

            printf("playerX's connection accepted\n");


            //read playerX PLAY step
            step = create_tttstep();
            if (read_tttstep(*(game->playerXSocket), step) == -1){
                printf("read_tttstep for X's PLAY failed, bad protocol\n");
                fflush(stdout);
                break;
            }

            if (strcmp(step->msgType, "PLAY") != 0){
                printf("expected PLAY step from playerX\n");
                fflush(stdout);
                break;
            }
            strcpy(game->playerXName, step->name);
            destroy_tttstep(step);


            //tell playerX to wait
            step = create_tttstep();
            strcpy(step->msgType, "WAIT");
            write_tttstep_WAIT(*(game->playerXSocket), step);
            destroy_tttstep(step);

            //accept playerO connection
            *(game->playerOSocket) = accept(queueSock, NULL, NULL);
            if (*(game->playerOSocket) == -1){
                printf("accept failed\n");
                fflush(stdout);
            }

            printf("playerO's connection accepted\n");



            //read playerO PLAY step
            step = create_tttstep();
            read_tttstep(*(game->playerOSocket), step);
            if (strcmp(step->msgType, "PLAY") != 0){
                printf("expected PLAY step from playerO\n");
                fflush(stdout);
            }
            strcpy(game->playerOName, step->name);
            destroy_tttstep(step);

            //tell playerO to wait
            step = create_tttstep();
            strcpy(step->msgType, "WAIT");
            write_tttstep_WAIT(*(game->playerOSocket), step);
            destroy_tttstep(step);

            //tell playerX to begin
            step = create_tttstep();
            strcpy(step->msgType, "BEGN");
            strcpy(step->role, "X");
            strcpy(step->name, game->playerOName);
            write_tttstep_BEGN(*(game->playerXSocket), step);
            destroy_tttstep(step);

            //tell playerO to begin
            step = create_tttstep();
            strcpy(step->msgType, "BEGN");
            strcpy(step->role, "O");
            strcpy(step->name, game->playerXName);
            write_tttstep_BEGN(*(game->playerOSocket), step);
            destroy_tttstep(step);
        }

        //game loop - setup game stuff for the thread, and create pthread for game
        strcpy(game->nextPlayer, "X");
        strcpy(game->board, ".........");
        pthread_create(game->gameThread, NULL, start_game, (void *) game);
    }

    printf("Exiting main.\n");
    fflush(stdout);

    close(queueSock);

    return 0;
}

// int main(int argc, char *argv[]){
//     tttstep *step;
//     char cbuff;
//     int readBytes = 0;

//     step = create_tttstep();

//     //WAIT test step
//     strcpy(step->msgType, "WAIT");
//     write_tttstep_WAIT(STDOUT_FILENO, step);
//     printf("\n");
//     fflush(stdout);

//     //BEGN test step
//     strcpy(step->msgType, "BEGN");
//     strcpy(step->role, "X");
//     strcpy(step->name, "David");
//     write_tttstep_BEGN(STDOUT_FILENO, step);
//     printf("\n");
//     fflush(stdout);

//     //MOVD test step
//     strcpy(step->msgType, "MOVD");
//     strcpy(step->role, "X");
//     strcpy(step->board, "XOXXOXXOO");
//     write_tttstep_MOVD(STDOUT_FILENO, step);
//     printf("\n");
//     fflush(stdout);

//     //INVL test step
//     strcpy(step->msgType, "INVL");
//     strcpy(step->reason, "Invalid move");
//     write_tttstep_INVL(STDOUT_FILENO, step);
//     printf("\n");
//     fflush(stdout);

//     //DRAW test step
//     strcpy(step->msgType, "DRAW");
//     strcpy(step->message, "A");
//     write_tttstep_DRAW(STDOUT_FILENO, step);
//     printf("\n");
//     fflush(stdout);

//     //OVER test step
//     strcpy(step->msgType, "OVER");
//     strcpy(step->outcome, "X");
//     strcpy(step->reason, "X won");
//     write_tttstep_OVER(STDOUT_FILENO, step);
//     printf("\n");
//     fflush(stdout);

//     destroy_tttstep(step);

//     return 0;
// }