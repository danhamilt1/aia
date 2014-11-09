CC = gcc
CFLAGS = -std=c99 -lncurses
DEPS = ws1.h
OBJ = ws1.o

%.o: %.c$(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

ws1: ws1.o
	$(CC) -o ws1 ws1.o -std=c99 -lncurses

clean:
	rm ./*.o
