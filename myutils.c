#include "myutils.h"

const char *VALID_READ_MSG_TYPES_STR[9] = {"PLAY", "MOVE", "RSGN", "DRAW"};
const char *VALID_WRITE_MSG_TYPES_STR[9] = {"WAIT", "BEGN", "MOVD", "INVL", "DRAW", "OVER"};

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
    game->gameThread = malloc(sizeof(pthread_t));
    game->gameOverType = malloc(1 * sizeof(int));
    game->statusXOLoseWin = malloc(3 * sizeof(char));
    game->activeNames = NULL;
    game->numActiveNames = NULL;
    game->activeNamesMutex = NULL;


    //check if malloc failed by checking if any of the pointers are NULL
    if (game->playerXSocket == NULL || 
        game->playerOSocket == NULL || 
        game->playerXName == NULL || 
        game->playerOName == NULL || 
        game->board == NULL || 
        game->nextPlayer == NULL ||
        game->gameThread == NULL ||
        game->gameOverType == NULL ||
        game->statusXOLoseWin == NULL){

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
    free(game->gameThread);
    free(game);
    return 0;
}




// read_tttstep reads a tttstep from the socket and stores it in the tttstep struct
// @returns 0 on success, -1 on failure
// @param sock the socket to read from
// @param step the tttstep struct to store the data in
int read_tttstep(int clientSock, tttstep *step){

    if (read_tttstep_msgType(clientSock, step) == -1){
        printf("read_tttstep_msgType failed\n");
        fflush(stdout);
        return -1;
    }

    if (read_tttstep_remainingBytes(clientSock, step) == -1){
        printf("read_tttstep_remainingBytes failed\n");
        fflush(stdout);
        return -1;
    }
    
    // branch off based on msg type
    if (step->msgType[0] == 'R'){
        if (read_tttstep_RSGN(clientSock, step) == -1){
            printf("read_tttstep_RSGN failed\n");
            fflush(stdout);
            return -1;
        }
        printf("RSGN received\n");
        fflush(stdout);
    }
    else if (step->msgType[0] == 'P'){
        if (read_tttstep_PLAY(clientSock, step) == -1){
            printf("read_tttstep_PLAY failed\n");
            fflush(stdout);
            return -1;
        }
        printf("PLAY received: %s\n", step->name);
        fflush(stdout);
    }
    else if (step->msgType[0] == 'M'){
        if (read_tttstep_MOVE(clientSock, step) == -1){
            printf("read_tttstep_MOVE failed\n");
            fflush(stdout);
            return -1;
        }
        printf("MOVE received: %d, %d\n", (step->position)[0], (step->position)[1]);
        fflush(stdout);
    }
    else if (step->msgType[0] == 'D'){
        if (read_tttstep_DRAW(clientSock, step) == -1){
            printf("read_tttstep_DRAW failed\n");
            fflush(stdout);
            return -1;
        }
        printf("DRAW received: %s\n", step->message);
        fflush(stdout);
    }
    else {
        printf("read_tttstep failed; msgType not RSGN, PLAY, MOVE, or DRAW\n");
        fflush(stdout);
        return -1;
    }
    return 0;
}

// read_tttstep helper method
// @returns 0 on valid msgType, -1 on invalid msgType
// @param sock the socket to read from, step the MALLOCED tttstep struct to store the msg in
int read_tttstep_msgType(int sock, tttstep *step){
    char readBuff[512];
    int msgTypeBytesRead = 0;
    int readBytes = 0;
    while (msgTypeBytesRead < 4){
        readBytes = read(sock, readBuff, 4 - msgTypeBytesRead);

        if (readBytes == -1){
            printf("read_tttstep_msgType's read failed\n");
            fflush(stdout);
            return -1;
        }

        if (readBytes == 0){
            printf("read_tttstep_msgType's read reached OEF\n");
            fflush(stdout);
            return -1;
        }

        readBuff[readBytes] = '\0';
        strcpy((step->msgType) + msgTypeBytesRead, readBuff);
        msgTypeBytesRead += readBytes;
    }

    // read '|'
    readBytes = read(sock, readBuff, 1);
    if (readBytes == -1){
        printf("read_tttstep_msgType's read failed\n");
        fflush(stdout);
        return -1;
    }
    if (readBytes == 0){
        printf("read_tttstep_msgType's read reached OEF\n");
        fflush(stdout);
        return -1;
    }
    if (readBuff[0] != '|'){
        printf("read_tttstep_msgType's read failed; expected '|'\n");
        fflush(stdout);
        return -1;
    }
    

    for (int i = 0; i < 4; i++){
        if (strcmp(step->msgType, VALID_READ_MSG_TYPES_STR[i]) == 0){
            return 0;
        }
    }

    printf("read_tttstep's read OK but invalid msgType");
    fflush(stdout);
    return -1; // invalid msgType
}

// read_tttstep helper method
int read_tttstep_remainingBytes(int sock, tttstep *step){
    char readString[5];
    int readBytes = 0;
    int numBytesRead = 0;
    char cbuff;

    while (numBytesRead < 4){
        readBytes = read(sock, &cbuff, 1);
        if (readBytes == -1){
            printf("read_tttstep_remainingBytes's read failed\n");
            fflush(stdout);
            return -1;
        }
        if (readBytes == 0){
            printf("read_tttstep_remainingBytes's read reached OEF\n");
            fflush(stdout);
            return -1;
        }

        readString[numBytesRead] = cbuff;
        numBytesRead += readBytes;
        if (cbuff == '|'){
            break;
        }
    }

    if (readString[numBytesRead - 1] != '|'){
        printf("read_tttstep_remainingBytes's read failed; expected '|'\n");
        fflush(stdout);
        return -1;
    }

    readString[numBytesRead - 1] = '\0';
    *(step->remainingBytes) = atoi(readString);
    if (*(step->remainingBytes) > 255 || *(step->remainingBytes) < 0){
        printf("read_tttstep_remainingBytes's read failed; too high or low remainingBytes\n");
        fflush(stdout);
        return -1;
    }
     return 0;
}

// read_tttstep helper method, reads rest of msg if RSGN
int read_tttstep_RSGN(int sock, tttstep *step){
    if (*(step->remainingBytes) != 0){
        printf("read_tttstep_RSGN's read failed; remainingBytes not 0\n");
        fflush(stdout);
        return -1;
    }

    return 0;
}

// read_tttstep helper method, reads rest of msg if PLAY
int read_tttstep_PLAY(int sock, tttstep *step){
    if (*(step->remainingBytes) == 1){
        printf("Name must be at least 1 character long\n");
        fflush(stdout);
        return -1;
    }

    if (*(step->remainingBytes) > 241){
        printf("Name must be less than 241 characters long\n");
        fflush(stdout);
        return -1;
    }

    char readBuff[512];
    int nameBytesRead = 0;
    int readBytes = 0;
    while (nameBytesRead < *(step->remainingBytes)){
        readBytes = read(sock, readBuff, *(step->remainingBytes) - nameBytesRead);

        if (readBytes == -1){
            printf("read_tttstep_PLAY's read failed\n");
            fflush(stdout);
            return -1;
        }

        if (readBytes == 0){
            printf("read_tttstep_PLAY's read reached OEF\n");
            fflush(stdout);
            return -1;
        }

        readBuff[readBytes] = '\0';
        strcpy((step->name) + nameBytesRead, readBuff);
        nameBytesRead += readBytes;
    }

    if (strchr(step->name, '|') != (step->name + *(step->remainingBytes) - 1)){
        printf("read_tttstep_PLAY's read failed; name contains '|' at an invalid position or not existant\n");
        fflush(stdout);
        return -1;
    }

    *(strchr(step->name, '|')) = '\0';

    return 0;
}

// read_tttstep helper method, reads rest of msg if MOVE
int read_tttstep_MOVE(int sock, tttstep *step){
    if (*(step->remainingBytes) != 6){
        printf("read_tttstep_MOVE's read failed; Remaining bytes not 6\n");
        fflush(stdout);
        return -1;
    }
    int readBytes = 0;
    int numBytesRead = 0;
    char cbuff;
    
    for (int i = -2; i < 4; i++){
        readBytes = read(sock, &cbuff, 1);
        if (readBytes == -1){
            printf("read_tttstep_MOVE's read failed\n");
            fflush(stdout);
            return -1;
        }
        if (readBytes == 0){
            printf("read_tttstep_MOVE's read reached OEF\n");
            fflush(stdout);
            return -1;
        }

        if (i == -2){
            if (cbuff != 'X' && cbuff != 'O'){
                printf("read_tttstep_MOVE's read failed; invalid player\n");
                fflush(stdout);
                return -1;
            } else {
                *(step->role) = cbuff;
                *(step->role + 1) = '\0';
            }
        }

        if (i == 0 || i == 2){
            if (cbuff < '1' || cbuff > '3'){
                printf("read_tttstep_MOVE's read failed; invalid coordinate\n");
                fflush(stdout);
                return -1;
            } else {
                if (i == 0){
                    (step->position)[0] = cbuff - '0';
                } else {
                    (step->position)[1] = cbuff - '0';
                }
            }
        }

        if (i == 1 && cbuff != ','){
            printf("read_tttstep_MOVE's read failed; expected ','\n");
            fflush(stdout);
            return -1;
        }
        if (i == 3 && cbuff != '|'){
            printf("read_tttstep_MOVE's read failed; expected '|'\n");
            fflush(stdout);
            return -1;
        }
    }
    return 0;
}

// read_tttstep helper method, reads rest of msg if DRAW
int read_tttstep_DRAW(int sock, tttstep *step){
    char cbuff;
    int bytesRead = 0;

    if (*(step->remainingBytes) != 2){
        printf("read_tttstep_DRAW's read failed; remainingBytes not 2\n");
        fflush(stdout);
        return -1;
    }

    for (int i = 0; i < 2; i++){
        bytesRead = read(sock, &cbuff, 1);
        if (bytesRead == -1){
            printf("read_tttstep_DRAW's read failed\n");
            fflush(stdout);
            return -1;
        }
        if (bytesRead == 0){
            printf("read_tttstep_DRAW's read reached OEF\n");
            fflush(stdout);
            return -1;
        }

        if (i == 0){
            if (cbuff != 'D' && cbuff != 'R' && cbuff != 'A'){
                printf("read_tttstep_DRAW's read failed; expected 'D' or 'R' or 'A' \n");
                fflush(stdout);
                return -1;
            } else {
                *(step->message) = cbuff;
                *(step->message + 1) = '\0';
            }
        } else if (i == 1){
            if (cbuff != '|'){
                printf("read_tttstep_DRAW's read failed; expected '|'\n");
                fflush(stdout);
                return -1;
            }
        }



    }

    return 0;
}




//write tttstep to socket
int write_tttstep(int sock, tttstep* step){
    if (step == NULL){
        printf("write_tttstep failed; step is NULL\n");
        fflush(stdout);
        return -1;
    } else {

        if (step->msgType[0] == 'W'){
            return write_tttstep_WAIT(sock, step);
        } else if (step->msgType[0] == 'B'){
            return write_tttstep_BEGN(sock, step);
        } else if (step->msgType[0] == 'M'){
            return write_tttstep_MOVD(sock, step);
        } else if (step->msgType[0] == 'I'){
            return write_tttstep_INVL(sock, step);
        } else if (step->msgType[0] == 'D'){
            return write_tttstep_DRAW(sock, step);
        } else if (step->msgType[0] == 'O'){
            return write_tttstep_OVER(sock, step);
        } else {
            printf("write_tttstep failed; step->type is invalid\n");
            fflush(stdout);
            return -1;
        }

    }
}

//write_tttstep helper method, writes WAIT
int write_tttstep_WAIT(int sock, tttstep* step){
    int writeBytes = 0;
    int numBytesWritten = 0;
    char *msgString = "WAIT|0|";
    *(step->remainingBytes) = 0;

    while (numBytesWritten < 7){
        writeBytes = write(sock, msgString + numBytesWritten, 7 - numBytesWritten);
        if (writeBytes == -1){
            printf("write_tttstep_WAIT's write failed\n");
            fflush(stdout);
            return -1;
        }
        if (writeBytes == 0){
            printf("write_tttstep_WAIT's write reached OEF\n");
            fflush(stdout);
            return -1;
        }

        numBytesWritten += writeBytes;
    }

    return 0;
}

//write_tttstep helper method, writes BEGN
int write_tttstep_BEGN(int sock, tttstep *step){
    char wbuff[512];
    *(step->remainingBytes) = strlen(step->name) + 3;
    

    int writeBytes = 0;
    int numBytesWritten = 0;

    wbuff[0] = '\0';
    strcat(wbuff, "BEGN|");
    
    sprintf(wbuff + strlen(wbuff), "%d", *(step->remainingBytes));
    strcat(wbuff, "|");
    strcat(wbuff, step->role);
    strcat(wbuff, "|");
    strcat(wbuff, step->name);
    strcat(wbuff, "|");

    int wbuffLength = strlen(wbuff);

    while (numBytesWritten < wbuffLength){
        writeBytes = write(sock, wbuff + numBytesWritten, wbuffLength - numBytesWritten);
        if (writeBytes == -1){
            printf("write_tttstep_BEGN's write failed\n");
            fflush(stdout);
            return -1;
        }
        if (writeBytes == 0){
            printf("write_tttstep_BEGN's write reached OEF\n");
            fflush(stdout);
            return -1;
        }

        numBytesWritten += writeBytes;
    }

    return 0;
}

//write_tttstep helper method, writes MOVD
int write_tttstep_MOVD(int sock, tttstep *step){

    char wbuff[512];
    wbuff[0] = '\0';
    *(step->remainingBytes) = strlen("X|N,N|.........|");
    
    strcat(wbuff, "MOVD|");
    sprintf(wbuff + strlen(wbuff), "%d", *(step->remainingBytes));
    strcat(wbuff, "|");
    strcat(wbuff, step->role);
    strcat(wbuff, "|");
    strcat(wbuff, step->board);
    strcat(wbuff, "|");

    int wbuffLength = strlen(wbuff);
    int writeBytes = 0;
    int numBytesWritten = 0;

    printf("write_tttstep_MOVD's wbuff: %s\n", wbuff);

    while (numBytesWritten < wbuffLength){
        writeBytes = write(sock, wbuff + numBytesWritten, wbuffLength - numBytesWritten);
        if (writeBytes == -1){
            printf("write_tttstep_MOVD's write failed\n");
            fflush(stdout);
            return -1;
        }
        if (writeBytes == 0){
            printf("write_tttstep_MOVD's write reached OEF\n");
            fflush(stdout);
            return -1;
        }

        numBytesWritten += writeBytes;
    }


    return 0;
}

//write_tttstep helper method, writes INVL
int write_tttstep_INVL(int sock, tttstep *step){
    char wbuff[512];
    *(step->remainingBytes) = strlen(step->reason) + 1;

    strcat(wbuff, "INVL|");
    sprintf(wbuff + strlen(wbuff), "%d", *(step->remainingBytes));
    strcat(wbuff, "|");
    strcat(wbuff, step->reason);
    strcat(wbuff, "|");

    int wbuffLength = strlen(wbuff);
    int writeBytes = 0;
    int numBytesWritten = 0;

    while (numBytesWritten < wbuffLength){
        writeBytes = write(sock, wbuff + numBytesWritten, wbuffLength - numBytesWritten);
        if (writeBytes == -1){
            printf("write_tttstep_INVL's write failed\n");
            fflush(stdout);
            return -1;
        }
        if (writeBytes == 0){
            printf("write_tttstep_INVL's write reached OEF\n");
            fflush(stdout);
            return -1;
        }

        numBytesWritten += writeBytes;
    }

    return 0;
}

//write_tttstep helper method, writes DRAW
int write_tttstep_DRAW(int sock, tttstep *step){
    char wbuff[512];
    *(step->remainingBytes) = strlen(step->message) + 1;

    strcat(wbuff, "DRAW|");
    sprintf(wbuff + strlen(wbuff), "%d", *(step->remainingBytes));
    strcat(wbuff, "|");
    strcat(wbuff, step->message);
    strcat(wbuff, "|");

    int wbuffLength = strlen(wbuff);
    int writeBytes = 0;
    int numBytesWritten = 0;

    while (numBytesWritten < wbuffLength){
        writeBytes = write(sock, wbuff + numBytesWritten, wbuffLength - numBytesWritten);
        if (writeBytes == -1){
            printf("write_tttstep_DRAW's write failed\n");
            fflush(stdout);
            return -1;
        }
        if (writeBytes == 0){
            printf("write_tttstep_DRAW's write reached OEF\n");
            fflush(stdout);
            return -1;
        }

        numBytesWritten += writeBytes;
    }

    return 0;
}

//write_tttstep helper method, writes OVER
int write_tttstep_OVER(int sock, tttstep *step){
    char wbuff[512];
    *(step->remainingBytes) = strlen(step->outcome) + strlen(step->reason) + 2;

    strcat(wbuff, "OVER|");
    sprintf(wbuff + strlen(wbuff), "%d", *(step->remainingBytes));
    strcat(wbuff, "|");
    strcat(wbuff, step->outcome);
    strcat(wbuff, "|");
    strcat(wbuff, step->reason);
    strcat(wbuff, "|");


    int wbuffLength = strlen(wbuff);
    int writeBytes = 0;
    int numBytesWritten = 0;

    while (numBytesWritten < wbuffLength){
        writeBytes = write(sock, wbuff + numBytesWritten, wbuffLength - numBytesWritten);
        if (writeBytes == -1){
            printf("write_tttstep_OVER's write failed\n");
            fflush(stdout);
            return -1;
        }
        if (writeBytes == 0){
            printf("write_tttstep_OVER's write reached OEF\n");
            fflush(stdout);
            return -1;
        }

        numBytesWritten += writeBytes;
    }

    return 0;
}

//start a game of tic tac toe
void * start_game(void *game_){
    tttgame *game = (tttgame *) game_;
    int activePlayerSocket = *(game->playerXSocket);
    char *activeRole = "X";

    while (1){
        if (game->nextPlayer[0] == 'X'){
            activePlayerSocket = *(game->playerXSocket);
            activeRole = "X";
        } else {
            activePlayerSocket = *(game->playerOSocket);
            activeRole = "O";
        }

        tttstep *step = create_tttstep();
        if (read_tttstep(activePlayerSocket, step) == -1){
            printf("read_tttstep failed in start_game\n");
            fflush(stdout);
            *(game->gameOverType) = 4;
            game_OVER(game, step);
        }

        if (strcmp(step->msgType, "MOVE") == 0){
            if (game_MOVE(game, step) == -1){
                printf("start_game_MOVE failed in start_game\n");
                fflush(stdout);
                *(game->gameOverType) = 4;
                game_OVER(game, step);
            }
        } else if (strcmp(step->msgType, "DRAW") == 0){
            if (game_DRAW(game, step) == -1){
                printf("start_game_DRAW failed in start_game\n");
                fflush(stdout);
                *(game->gameOverType) = 4;
                game_OVER(game, step);
            }
        } else if (strcmp(step->msgType, "RSGN") == 0){
            printf("%s RSGNed\n", activeRole);
            fflush(stdout);
            *(game->gameOverType) = 3;
            game_OVER(game, step);
        } else if (strcmp(step->msgType, "PLAY") == 0){
            printf("PLAY should not be in start_game\n");
            fflush(stdout);
            *(game->gameOverType) = 4;
            game_OVER(game, step);
        } else {
            printf("Invalid msgType in start_game. This should be impossible to reach\n");
            fflush(stdout);
            *(game->gameOverType) = 4;
            game_OVER(game, step);
        }
    }
}

//start_game helper function, executes move, @returs 0 if OK, -1 if fatal error, 1 = X won, 2 = O won
int game_MOVE(tttgame *game, tttstep *step){
    int coordsToIndex = 0;
    tttstep *tempStep = create_tttstep();
    
    coordsToIndex = ((step->position[0] - 1) * 3) + (step->position[1] - 1);
    if (game->board[coordsToIndex] == '.' && step->role[0] == game->nextPlayer[0]){
        game->board[coordsToIndex] = step->role[0];
    } else {
        strcpy(tempStep->msgType, "INVL");
        *(tempStep->remainingBytes) = 51;
        strcpy(tempStep->reason, "Invalid move, position already taken or wrong role");
        if (step->role[0] == 'X'){
            if (write_tttstep_INVL(*(game->playerXSocket), tempStep) == -1){
                printf("write_tttstep_INVL failed in start_game_MOVE\n");
                fflush(stdout);
                return -1;
            }
        } else {
            if (write_tttstep_INVL(*(game->playerOSocket), tempStep) == -1){
                printf("write_tttstep_INVL failed in start_game_MOVE\n");
                fflush(stdout);
                return -1;
            }
        }

        return 0;
    }

    //update clients MOVD
    strcpy(tempStep->msgType, "MOVD");
    *(tempStep->remainingBytes) = 6;
    strcpy(tempStep->role, step->role);
    strcpy(tempStep->board, game->board);
    memcpy(tempStep->position, step->position, 2*sizeof(int));
    if (write_tttstep_MOVD(*(game->playerXSocket), tempStep) == -1){
        printf("write_tttstep_MOVD failed in start_game_MOVE\n");
        fflush(stdout);
        return -1;
    }
    if (write_tttstep_MOVD(*(game->playerOSocket), tempStep) == -1){
        printf("write_tttstep_MOVD failed in start_game_MOVE\n");
        fflush(stdout);
        return -1;
    }
    
    //check for win
    for (int i = 0; i < 3; i++){
        if (game->board[i] == game->board[i+3] && game->board[i+3] == game->board[i+6] && game->board[i] != '.'){
            //win
            if (game->board[i] == 'X'){
                *(game->gameOverType) = 1;
                strcpy(game->statusXOLoseWin, "WL");
                game_OVER(game, step);
            } else {
                *(game->gameOverType) = 1;
                strcpy(game->statusXOLoseWin, "LW");
                game_OVER(game, step);
            }
        }
        if (game->board[i*3] == game->board[i*3+1] && game->board[i*3+1] == game->board[i*3+2] && game->board[i*3] != '.'){
            //win
            if (game->board[i*3] == 'X'){
                *(game->gameOverType) = 1;
                strcpy(game->statusXOLoseWin, "WL");
                game_OVER(game, step);
            } else {
                *(game->gameOverType) = 1;
                strcpy(game->statusXOLoseWin, "LW");
                game_OVER(game, step);
            }
        }
        if (game->board[0] == game->board[4] && game->board[4] == game->board[8] && game->board[0] != '.'){
            //win
            if (game->board[0] == 'X'){
                *(game->gameOverType) = 1;
                strcpy(game->statusXOLoseWin, "WL");
                game_OVER(game, step);
            } else {
                *(game->gameOverType) = 1;
                strcpy(game->statusXOLoseWin, "LW");
                game_OVER(game, step);
            }
        }
        if (game->board[2] == game->board[4] && game->board[4] == game->board[6] && game->board[2] != '.'){
            //win
            if (game->board[2] == 'X'){
                *(game->gameOverType) = 1;
                strcpy(game->statusXOLoseWin, "WL");
                game_OVER(game, step);
            } else {
                *(game->gameOverType) = 1;
                strcpy(game->statusXOLoseWin, "LW");
                game_OVER(game, step);
            }
        }
    }

    //check for draw
    if (strchr(game->board, '.') == NULL){
        //draw
        *(game->gameOverType) = 1;
        strcpy(game->statusXOLoseWin, "DD");
        game_OVER(game, step);
    }



    return 0;

}

// start_game helper function, executes draw, @returs 0 if OK, -1 if fatal error
int game_DRAW(tttgame *game, tttstep *step){
    tttstep *sharedStep = create_tttstep();
    int starterSocket = 0;
    int endingSocket = 0;
    if (game->nextPlayer[0] == 'X'){
        starterSocket = *(game->playerXSocket);
        endingSocket = *(game->playerOSocket);
    } else {
        starterSocket = *(game->playerOSocket);
        endingSocket = *(game->playerXSocket);
    }

    if (step->message[0] != 'S'){
        strcpy(sharedStep->msgType, "INVL");
        *(sharedStep->remainingBytes) = 53;
        strcpy(sharedStep->reason, "Invalid draw, you cannot start a DRAW by with A or R");
        if (write_tttstep_INVL(starterSocket, sharedStep) == -1){
            printf("write_tttstep_INVL failed in start_game_DRAW\n");
            fflush(stdout);
            return -1;
        }
        return 0;
    }

    strcpy(sharedStep->msgType, "DRAW");
    *(sharedStep->remainingBytes) = 2;
    strcpy(sharedStep->message, "S");
    if (write_tttstep_DRAW(endingSocket, sharedStep) == -1){
        printf("write_tttstep_DRAW failed in start_game_DRAW\n");
        fflush(stdout);
        return -1;
    }

    if (read_tttstep_DRAW(endingSocket, sharedStep) == -1){
        printf("read_tttstep_DRAW failed in start_game_DRAW\n");
        fflush(stdout);
        return -1;
    }

    while (1){
        if (read_tttstep_DRAW(endingSocket, sharedStep) == -1){
            printf("read_tttstep_DRAW failed in start_game_DRAW\n");
            fflush(stdout);
            return -1;
        }
        if (sharedStep->message[0] != 'A' && sharedStep->message[0] != 'R'){
            tttstep *invalidStep = create_tttstep();
            strcpy(invalidStep->msgType, "INVL");
            *(invalidStep->remainingBytes) = 53;
            strcpy(invalidStep->reason, "Invalid draw, you must reply to a DRAW S with A or R");
            if (write_tttstep_INVL(endingSocket, invalidStep) == -1){
                printf("write_tttstep_INVL failed in start_game_DRAW\n");
                fflush(stdout);
                return -1;
            }
        }
        if (sharedStep->message[0] == 'A'){
            *(game->gameOverType) = 2;
            strcpy(game->statusXOLoseWin, "DD");
            game_OVER(game, step);
        }
        if (sharedStep->message[0] == 'R'){
            tttstep *replyStep = create_tttstep();
            strcpy(replyStep->msgType, "DRAW");
            *(replyStep->remainingBytes) = 2;
            strcpy(replyStep->reason, "R");
            if (write_tttstep_DRAW(endingSocket, replyStep) == -1){
                printf("write_tttstep_INVL failed in start_game_DRAW\n");
                fflush(stdout);
                return -1;
            }
            return 0;
        }
    }
}

int game_OVER(tttgame *game, tttstep *step){
    if (*(game->gameOverType) == 1){
        //natural outcome
        tttstep *overStep = create_tttstep();
        strcpy(overStep->msgType, "OVER");
        *(overStep->remainingBytes) = 46 + 2;
        strcpy(overStep->reason, "Natural outcome: Three-in-a-row or board full");
        *(overStep->outcome) = game->statusXOLoseWin[0];
        if (write_tttstep_OVER(*(game->playerXSocket), overStep) == -1){
            printf("write_tttstep_OVER failed in start_game_OVER\n");
            fflush(stdout);
            return -1;
        }
        *(overStep->outcome) = game->statusXOLoseWin[1];
        if (write_tttstep_OVER(*(game->playerOSocket), overStep) == -1){
            printf("write_tttstep_OVER failed in start_game_OVER\n");
            fflush(stdout);
            return -1;
        }
        close(*(game->playerXSocket));
        close(*(game->playerOSocket));
        destroy_tttstep(overStep);
        destroy_tttgame(game);
        destroy_tttstep(step);
        pthread_exit(NULL);
    } else if (*(game->gameOverType) == 2){
        // premature DRAW
        tttstep *overStep = create_tttstep();
        strcpy(overStep->msgType, "OVER");
        *(overStep->remainingBytes) = 17 + 2;
        strcpy(overStep->reason, "Agreed upon DRAW");
        *(overStep->outcome) = 'D';
        if (write_tttstep_OVER(*(game->playerXSocket), overStep) == -1){
            printf("write_tttstep_OVER failed in start_game_OVER\n");
            fflush(stdout);
            return -1;
        }
        if (write_tttstep_OVER(*(game->playerOSocket), overStep) == -1){
            printf("write_tttstep_OVER failed in start_game_OVER\n");
            fflush(stdout);
            return -1;
        }
        close(*(game->playerXSocket));
        close(*(game->playerOSocket));
        destroy_tttstep(overStep);
        destroy_tttgame(game);
        destroy_tttstep(step);
        pthread_exit(NULL);
    } else if (*(game->gameOverType) == 3){
        tttstep *overStep = create_tttstep();
        strcpy(overStep->msgType, "OVER");
        *(overStep->remainingBytes) = 17 + 2;
        if (*(game->statusXOLoseWin) == 'L'){
            strcpy(overStep->reason, "Player X resigned");
        } else {
            strcpy(overStep->reason, "Player O resigned");
        }
        *(overStep->outcome) = game->statusXOLoseWin[0];
        if (write_tttstep_OVER(*(game->playerXSocket), overStep) == -1){
            printf("write_tttstep_OVER failed in start_game_OVER\n");
            fflush(stdout);
            return -1;
        }
        *(overStep->outcome) = game->statusXOLoseWin[1];
        if (write_tttstep_OVER(*(game->playerOSocket), overStep) == -1){
            printf("write_tttstep_OVER failed in start_game_OVER\n");
            fflush(stdout);
            return -1;
        }
        close(*(game->playerXSocket));
        close(*(game->playerOSocket));
        destroy_tttstep(overStep);
        destroy_tttgame(game);
        destroy_tttstep(step);
        pthread_exit(NULL);
    } else if (*(game->gameOverType) == 4){
        tttstep *overStep = create_tttstep();
        strcpy(overStep->msgType, "OVER");
        strcpy(overStep->outcome, "D");
        *(overStep->remainingBytes) = 41 + 2;
        strcpy(overStep->reason, "Malformed message according to protocol!");
        if (write_tttstep_OVER(*(game->playerXSocket), overStep) == -1){
            printf("write_tttstep_OVER failed in start_game_OVER\n");
            fflush(stdout);
            return -1;
        }
        if (write_tttstep_OVER(*(game->playerOSocket), overStep) == -1){
            printf("write_tttstep_OVER failed in start_game_OVER\n");
            fflush(stdout);
            return -1;
        }
        close(*(game->playerXSocket));
        close(*(game->playerOSocket));
        destroy_tttstep(overStep);
        destroy_tttgame(game);
        destroy_tttstep(step);
        pthread_exit(NULL);
    }

    printf("How did you get here?\n");
    fflush(stdout);
    close(*(game->playerXSocket));
    close(*(game->playerOSocket));
    destroy_tttgame(game);
    destroy_tttstep(step);
    pthread_exit(NULL);
    
}