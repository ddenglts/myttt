#ifndef _MYUTILS_H
#define _MYUTILS_H
#define _POSIX_C_SOURCE 200809L


#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netdb.h>
#include <pthread.h>
#include <netinet/tcp.h>

typedef struct _tttstep {
    char *msgType; // 4 chars string + 1 null terminator
    int *remainingBytes; // 1 int, should be < 255
    char *name; // string
    char *role; // 1 char, X or O + 1 null terminator
    int *position; // 2 ints, [0] is row #, [1] is col #
    char *board; // 9 chars, 3 rows of 3 chars each, + 1 null terminator
    char *reason; // string
    char *message; // 1 char, S or A or R + 1 null terminator
    char *outcome; // 1 char, W or L or D + 1 null terminator
} tttstep;

typedef struct _tttgame {
    int *playerXSocket;
    int *playerOSocket;
    char *playerXName;
    char *playerOName;
    char *board;
    char *nextPlayer;

    // game phase stuff
    int *gameOverType; // 1 = natural W/D/L, 2 = agreed DRAW, 3 = RESGN, 4 = bad protocol
    char *statusXOLoseWin;

    //to deal with threading stuff
    pthread_t *gameThread;
    char **activeNames;
    int *numActiveNames;
    pthread_mutex_t *activeNamesMutex;


} tttgame;


typedef char mallocedchar; // dynamic char; created with malloc
typedef int mallocedint; // dynamic int; created with malloc

extern const char *VALID_READ_MSG_TYPES_STR[9];
extern const char *VALID_WRITE_MSG_TYPES_STR[9];



int err_die(const char *str);
tttstep* create_tttstep(void);
int destroy_tttstep(tttstep *step);
tttgame* create_tttgame(void);
int destroy_tttgame(tttgame *game);



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
void * start_game(void *game_);
int game_OVER(tttgame *game, tttstep *step);
int game_MOVE(tttgame *game, tttstep *step);
int game_DRAW(tttgame *game, tttstep *step);




#endif