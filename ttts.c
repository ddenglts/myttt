// NOTE: must use option -pthread when compiling!
#define _POSIX_C_SOURCE 200809L
#include "myutils.h"

int open_listener(char *portStr, int queueSize);
int read_tttstep(int sock, tttstep *step);
int read_tttstep_msgType(int sock, tttstep *step);
int read_tttstep_remainingBytes(int sock, tttstep *step);
int read_tttstep_PLAY(int sock, tttstep *step);
int read_tttstep_MOVE(int sock, tttstep *step);
int read_tttstep_RSGN(int sock, tttstep *step);
int read_tttstep_DRAW(int sock, tttstep *step);
int write_tttstep(int sock, tttstep *step);
int write_tttstep_WAIT(int sock, tttstep *step);
int write_tttstep_BEGN(int sock, tttstep *step);
int write_tttstep_MOVD(int sock, tttstep *step);
int write_tttstep_INVL(int sock, tttstep *step);
int write_tttstep_DRAW(int sock, tttstep *step);
int write_tttstep_OVER(int sock, tttstep *step);


#define QUEUE_SIZE 8
const char *VALID_READ_MSG_TYPES_STR[9] = {"PLAY", "MOVE", "RSGN", "DRAW"};
const char *VALID_WRITE_MSG_TYPES_STR[9] = {"WAIT", "BEGN", "MOVD", "INVL", "DRAW", "OVER"};




// read_tttstep reads a tttstep from the socket and stores it in the tttstep struct
// @returns 0 on success, -1 on failure
// @param sock the socket to read from
// @param step the tttstep struct to store the data in
int read_tttstep(int clientSock, tttstep *step){
    step = create_tttstep();

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
    if (*(step->remainingBytes) != 4){
        printf("read_tttstep_MOVE's read failed; Remaining bytes not 4\n");
        fflush(stdout);
        return -1;
    }
    int readBytes = 0;
    int numBytesRead = 0;
    char cbuff;
    
    for (int i = 0; i < 4; i++){
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



//write tttstep to socket
int write_tttstep(int sock, tttstep* step){
    return 0;
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
    if (step->msgType != "MOVD"){
        printf("write_tttstep_MOVD's write failed; msgType not MOVD\n");
        fflush(stdout);
        return -1;
    }

    char wbuff[512];
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



int create_game_instance(tttgame* game){ // CLEANS UP AFTER MAIN(), THIS FUNCITON IS THE FINAL ENDPOINT

}

//------------------MAIN------------------

// int main(int argc, char *argv[]){
//     int queueSock, clientSock;
//     tttstep *step;

//     queueSock = open_listener(argv[1], QUEUE_SIZE);
//     if (queueSock == -1){
//         err_die("open_listener failed");
//     }

//     for(;;){
//         clientSock = accept(queueSock, NULL, NULL);
//         if (clientSock == -1){
//             printf("accept failed\n");
//             fflush(stdout);
//         }


//         for (;;){
            
//         }

//         printf("closing clientSock\n");
//         fflush(stdout);
//         close(clientSock);
//     }

//     close(queueSock);

//     return 0;
// }

int main(int argc, char *argv[]){
    tttstep *step;
    char cbuff;
    int readBytes = 0;

    step = create_tttstep();
    printf("step created addr %p\n", step);
    fflush(stdout);

    //WAIT test step
    strcpy(step->msgType, "WAIT");
    write_tttstep_WAIT(STDOUT_FILENO, step);
    printf("\n");
    fflush(stdout);

    //BEGN test step
    strcpy(step->msgType, "BEGN");
    strcpy(step->role, "X");
    strcpy(step->name, "David");
    write_tttstep_BEGN(STDOUT_FILENO, step);
    printf("\n");
    fflush(stdout);

    //MOVD test step
    step->msgType = "MOVD";
    step->role = "X";
    step->board = ".........";
    write_tttstep_MOVD(STDOUT_FILENO, step);
    printf("\n");
    fflush(stdout);

    //INVL test step
    step->msgType = "INVL";
    step->reason = "invalid move";
    write_tttstep_INVL(STDOUT_FILENO, step);
    printf("\n");
    fflush(stdout);

    //DRAW test step
    step->msgType = "DRAW";
    step->message = "A";
    write_tttstep_DRAW(STDOUT_FILENO, step);
    printf("\n");
    fflush(stdout);

    //OVER test step
    step->msgType = "OVER";
    step->outcome = "W";
    step->reason = "X won";
    write_tttstep_OVER(STDOUT_FILENO, step);
    printf("\n");
    fflush(stdout);

    printf("step destroying addr %p\n", step);
    fflush(stdout);

    destroy_tttstep(step);

    return 0;
}