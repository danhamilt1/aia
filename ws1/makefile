CC = gcc
CFLAGS = -std=c99 -l ncurses -O3 -pthread
DEPS = ws1.h
OBJ = ws1.o

%.o: %.c$(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

ws1: ws1.o
	$(CC) -o ws1 ws1.o -std=c99 -l ncurses -O3 -pthread

clean:
	rm ./*.o
