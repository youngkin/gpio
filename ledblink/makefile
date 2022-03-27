matrixlednolib: matrixlednolib.c bcmfuncs.h
		gcc -g -o matrixlednolib matrixlednolib.c bcmfuncs.o

blinkinglednolib: blinkinglednolib.c bcmfuncs.h
		gcc -g -o blinkinglednolib blinkinglednolib.c bcmfuncs.o

blinkingled: blinkingled.c bcmfuncs.h
		gcc blinkingled.c -o blinkingled -lwiringPi

bcmlib: bcmfuncs.h bcmfuncs.c
		gcc -c -g bcmfuncs.c

clean: 
		rm -rf bcmfuncs.o matrixlednolib blinkinglednolib blinkingled

all: bcmlib matrixlednolib blinkingled blinkinglednolib
