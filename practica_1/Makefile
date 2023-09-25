CC=gcc
DEPS=-lpthread

practica1: practica1.c
	$(CC) -o practica1 practica1.c $(DEPS)

debug: clean practica1.c
	$(CC) -o practica1 practica1.c $(DEPS) -DDEBUG

clean:
	rm practica1
