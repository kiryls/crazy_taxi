CFLAGS = -std=c89 -pedantic


all: master child reader

clean:
	rm -f *.o master child reader *~

master: master.c common.h makefile
	gcc $(CFLAGS) master.c -o master

child: child.c common.h makefile
	gcc $(CFLAGS) child.c -o child

reader: reader.c common.h makefile
	gcc $(CFLAGS) reader.c -o reader


run:  all
	./master



