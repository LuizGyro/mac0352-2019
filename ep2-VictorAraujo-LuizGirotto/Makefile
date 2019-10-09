CC=gcc
CFLAGS=-Wall -Wextra -Werror -pthread
OBJ = immortal.o leader.o worker.o file.o util.o lln.o llip.o

ep2: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

immortal.o: immortal.c immortal.h worker.h leader.h file.h util.h lln.h llip.h
	$(CC) -c -o $@ $< $(CFLAGS)

worker.o: worker.c worker.h leader.h file.h util.h lln.h llip.h
	$(CC) -c -o $@ $< $(CFLAGS)

leader.o: leader.c leader.h file.h util.h lln.h llip.h
	$(CC) -c -o $@ $< $(CFLAGS)

file.o: file.c file.h
	$(CC) -c -o $@ $< $(CFLAGS)

util.o: util.c util.h
	$(CC) -c -o $@ $< $(CFLAGS)

lln.o: lln.c lln.h
	$(CC) -c -o $@ $< $(CFLAGS)

llip.o: llip.c llip.h
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm ep2
	rm *.o
	rm split*
