CFLAGS = -std=c89 -pedantic

all: master source #taxi

clean:
	rm -f objs/*.o objs/master objs/child  objs/*~

master: src/master.c headers/common.h makefile
	gcc $(CFLAGS) src/master.c -o objs/master

source: src/source.c headers/common.h makefile
	gcc $(CFLAGS) src/source.c -o objs/source


run:  all
	./master



