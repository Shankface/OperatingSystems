all: tas.o spinlock.o 2a.o 2a.out sem.o 2c.o 2c.out
.PHONY: clean 

tas.o: tas.S tas.h
	gcc -c tas.S -o tas.o

spinlock.o: tas.h spinlock.c spinlock.h
	gcc -c spinlock.c -o spinlock.o

2a.o: 2a.c tas.h spinlock.h
	gcc -c 2a.c -o 2a.o

2a.out: 2a.o tas.o spinlock.o
	gcc 2a.o tas.o spinlock.o -o 2a.out

sem.o: sem.c sem.h tas.h spinlock.h
	gcc -c sem.c

2c.o: 2c.c tas.h spinlock.h sem.h
	gcc -c 2c.c -o 2c.o

2c.out: 2c.o tas.o spinlock.o sem.o
	gcc 2c.o tas.o spinlock.o sem.o -o 2c.out

clean:
	rm -f *.o
	rm -f *.out