CC = gcc
CFLAGS = -std=c99
DEPS = ws1.h
OBJ = ws1.o

%.o: %.c$(DEPS)
	$(CC) -c -o $@ $< $(CFLAGG)

ws1: ws1.o
	$(CC) -o ws1 ws1.o -std=c99

clean:
	rm ./*.o
