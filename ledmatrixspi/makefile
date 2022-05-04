leddotmatrixnolib: leddotmatrixnolib.c ../ledblink/bcmfuncs.o
		gcc -g -o leddotmatrixnolib leddotmatrixnolib.c ../ledblink/bcmfuncs.o

leddotmatrix: leddotmatrix.c 
		gcc -o leddotmatrix leddotmatrix.c  -lbcm2835

bcmlib: ../ledblink/bcmfuncs.h ../ledblink/bcmfuncs.c
		gcc -c -g ../ledblink/bcmfuncs.c -o ../ledblink/bcmfuncs.o

clean: 
		rm -rf ../ledblink/bcmfuncs.o leddotmatrix leddotmatrixnolib 

all: bcmlib leddotmatrixnolib leddotmatrix 
