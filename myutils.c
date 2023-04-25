#include "myutils.h"

int err_die(const char *str){
    if (errno != 0) {
        perror(str);
        fprintf(stderr, ", CUSTOM DESC: %s\n", str);
        fflush(stderr);
    } else {
        fprintf(stderr, "CUSTOM ERROR (errno = 0): %s\n", str);
        fflush(stderr);
    }


    exit(1);
}

tttstep* create_tttstep(void){
    tttstep *step = malloc(sizeof(tttstep));
    step->msgType = malloc(5 * sizeof(char));
    step->remainingBytes = malloc(1 * sizeof(int));
    step->name = malloc(256 * sizeof(char));
    step->role = malloc( 2 * sizeof(char));
    step->position = malloc(2 * sizeof(int));
    step->board = malloc(10 * sizeof(char));
    step->reason = malloc(256 * sizeof(char));
    step->message = malloc(2 * sizeof(char));
    step->outcome = malloc(2 * sizeof(char));
    
    //check if malloc failed by checking if any of the pointers are NULL
    if (step->msgType == NULL || 
        step->remainingBytes == NULL || 
        step->name == NULL || 
        step->role == NULL || 
        step->position == NULL || 
        step->board == NULL || 
        step->reason == NULL || 
        step->message == NULL || 
        step->outcome == NULL){

        printf("EXTREMELY FATAL ERROR: malloc failed\n");
        fflush(stdout);
        exit(EXIT_FAILURE);
    }

    return step;
}

int destroy_tttstep(tttstep *step){
    free(step->msgType);
    free(step->remainingBytes);
    free(step->name);
    free(step->role);
    free(step->position);
    free(step->board);
    free(step->reason);
    free(step->message);
    free(step->outcome);
    free(step);
    return 0;
}

tttgame * create_tttgame(void){
    tttgame *game = malloc(sizeof(tttgame));
    game->playerXSocket = malloc(sizeof(int));
    game->playerOSocket = malloc(sizeof(int));
    game->playerXName = malloc(256 * sizeof(char));
    game->playerOName = malloc(256 * sizeof(char));
    game->board = malloc(10 * sizeof(char));
    game->nextPlayer = malloc(2 * sizeof(char));

    //check if malloc failed by checking if any of the pointers are NULL
    if (game->playerXSocket == NULL || 
        game->playerOSocket == NULL || 
        game->playerXName == NULL || 
        game->playerOName == NULL || 
        game->board == NULL || 
        game->nextPlayer == NULL){

        printf("EXTREMELY FATAL ERROR: malloc failed\n");
        fflush(stdout);
        exit(EXIT_FAILURE);
    }

    return game;
}

int destroy_tttgame(tttgame *game){
    free(game->playerXSocket);
    free(game->playerOSocket);
    free(game->playerXName);
    free(game->playerOName);
    free(game->board);
    free(game->nextPlayer);
    free(game);
    return 0;
}