CC = gcc
CFLAGS = -std=gnu99 -l ncurses -O3
DEPS = ws1.h
OBJ = ws1.o

%.o: %.c$(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

ws1: ws1.o
	$(CC) -o ws1 ws1.o -std=gnu99 -l ncurses -O3

clean:
	rm ./*.o
