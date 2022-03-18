##lednolib: blinkinglednolib.o bcmfuncs.o
##		gcc -o blinkinglednolib blinkinglednolib.o bcmfuncs.o

lednolib: blinkinglednolib.c bcmfuncs.h
		gcc -g -o lednolib blinkinglednolib.c bcmfuncs.o

bcmlib: bcmfuncs.h bcmfuncs.c
		gcc -c -g bcmfuncs.c

clean: 
		rm -rf bcmfuncs.o lednolib

all: bcmlib lednolib
