CFLAGS = -std=c89 -pedantic

all: master source taxi timer

clean:
	ipcrm -a
	rm -f out/*.o out/master out/source out/taxi out/*~ logs/*

run:  all
	./out/master

###############################################################

master.o: src/master.c
	gcc $(CFLAGS) -c src/master.c -o out/master.o

master: common.o master.o 
		gcc $(CFLAGS) -o out/master out/master.o out/common.o 

###############################################################

source.o: src/source.c
	gcc $(CFLAGS) -c src/source.c -o out/source.o

source: common.o source.o 
	gcc $(CFLAGS) -o out/source out/source.o out/common.o

###############################################################

taxi.o: src/taxi.c
	gcc $(CFLAGS) -c src/taxi.c -o out/taxi.o

taxi: common.o taxi.o
	gcc $(CFLAGS) -o out/taxi out/taxi.o out/common.o

###############################################################

timer.o: src/timer.c
	gcc $(CFLAGS) -c src/timer.c -o out/timer.o

timer: common.o timer.o
	gcc $(CFLAGS) -o out/timer out/timer.o out/common.o

###############################################################

common.o: src/common.c
	gcc $(CFLAGS) -c src/common.c -o out/common.o



