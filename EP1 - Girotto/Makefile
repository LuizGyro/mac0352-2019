CC=gcc
CFLAGS=-ansi -Wall -pedantic -g -O2

ep: ep1-servidor-exemplo.c
	$(CC) $(CFLAGS) ep1-servidor-exemplo.c -o servidor

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -rf *.o
