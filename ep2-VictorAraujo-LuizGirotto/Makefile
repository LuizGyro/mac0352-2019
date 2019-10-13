CC=gcc
FLAGS=-Wall -Wextra -pthread
OBJ=ep02.c immortal.o worker.o leader.o file.o util.o lln.o llip.o

ep2: $(OBJ)
	$(CC) $(OBJ) $(FLAGS) -o $@

.c.o:
	$(CC) $< $(FLAGS) -c

clean:
	rm -f ep2
	rm -f *.o
	rm -f split*
	rm -f result.txt
	rm -f log*
	killall -9 ep2
