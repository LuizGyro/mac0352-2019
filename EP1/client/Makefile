CC=gcc
CFLAGS=-ansi -Wall -pedantic -g -O2

ep: aula07-cliente-ftp.c
	$(CC) $(CFLAGS) aula07-cliente-ftp.c -o cliente

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -rf *.o
