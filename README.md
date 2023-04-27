# Creator: djd308 (David Deng), _NO PARTNER_

# HOW TO MAKE THE PROGRAMS
    ttts: make
    ttts: make ttts
    ttt: make ttt

# IMPLEMENTATION NOTES
    CONCURRENT GAMES using pthreads and mutex have been implemented

# Lock Use
    I have a list of names, number of names, and a mutex. 
    Operations concerning these shared resources are interacted with safely through a very bare interface using "addName()" and "delName()".
    They are in myutils.h, and defined in myutils.c.

# Test Plan
    5 big things to test:
        1. Handling bad protocol interaction
        2. Resign mechanism
        3. Agreed draw machanism
        4. Natural game outcome, three-in-a-row or board full
        5. Unique names


## 1. Handling bad protocol interaction
    input:
        MOVE|3|AS|
    output:
        [server says that it was expecting PLAY]

    input:
        PLAY|3|AS|
        MOVE|6|X|2,2|
        MOVE|6|X|2,2|
    output:
        [server says invalid move, already taken, sends INVL]

## 2. Resign mechanism
    _note: ... = assume normal input_
    input:
        ...
        RSGN|O|
    output:
        [OVER to both clients, reports X resigning, closes game and thread]

## 3. Agreed draw machanism
    input:
        ...
        DRAW|2|S| (by X)
        DRAW|2|A| (by O)
    output:
        [sends DRAW|2|S| to O, recvs input from O and then sends OVER to both]

    input:
        ...
        DRAW|2|S| (by X)
        DRAW|2|R| (by O)
    output:
        [sends DRAW|2|S| to O, recvs input from O and then listens to X]

## 4. Natural game outcome, three-in-a-row or board full
    input:
        ...
        MOVE|6|X|1,1|
        MOVE|6|X|1,2|
        MOVE|6|X|1,3|
    output:
        [sends MOVD to both each MOVE, then OVER as three-in-row detected]

## 5. Unique names
    input:
        PLAY|3|qw|
        PLAY|3|qw|
    output:
        [reports that name is already taken]