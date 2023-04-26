server: ttts.c myutils.c myutils.h
	gcc -g -o sserv ttts.c myutils.c -pthread

client: ttt.c myutils.c myutils.h
	gcc -g -o sclient ttt.c myutils.c -pthread
