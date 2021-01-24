CFLAGS = -std=c89 -pedantic

all: master source #taxi

clean:
	rm -f out/*.o out/master out/child  out/*~

master: src/master.c headers/common.h makefile
	gcc $(CFLAGS) src/master.c -o out/master

source: src/source.c headers/common.h makefile
	gcc $(CFLAGS) src/source.c -o out/source


run:  all
	./src/master



