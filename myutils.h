#ifndef _MYUTILS_H
#define _MYUTILS_H



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
#include <pthread.h>
#include <netdb.h>

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
} tttgame;


typedef char mallocedchar; // dynamic char; created with malloc
typedef int mallocedint; // dynamic int; created with malloc



int err_die(const char *str);
tttstep* create_tttstep(void);
int destroy_tttstep(tttstep *step);
tttgame* create_tttgame(void);
int destroy_tttgame(tttgame *game);



#endif