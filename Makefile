ttts: ttts.c myutils.c myutils.h
	gcc -g -o ttts ttts.c myutils.c -pthread

ttt: ttt.c myutils.c myutils.h
	gcc -g -o ttt ttt.c myutils.c -pthread
