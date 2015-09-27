CC=gcc
CFLAGS=-std=c11 -O3 -ggdb3 -march=native

LIBS=

DEPS=clenshaw.h

OBJ=clenshaw.o main.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

clenshaw: $(OBJ)
	gcc -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f *.o clenshaw
