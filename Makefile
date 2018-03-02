
all: aeo

aeo: wait.o
	gcc -O2 -o aeo wait.o

wait.o: wait.c
	gcc -O2 -Wall -c wait.c

clean:
	rm -f wait.o

distclean:
	make clean
	rm -f aeo
