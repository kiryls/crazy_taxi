CFLAGS = -std=c89 -pedantic

all: master source 

clean:
	ipcrm -a
	rm -f out/*.o out/master out/source  out/*~

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

common.o: src/common.c
	gcc $(CFLAGS) -c src/common.c -o out/common.o

###############################################################




