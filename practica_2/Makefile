CC=gcc
DEPS=-lpthread

cyclictestURJC: cyclictestURJC.c
	$(CC) -o cyclictestURJC cyclictestURJC.c $(DEPS)

debug: clean cyclictestURJC.c
	$(CC) -o cyclictestURJC cyclictestURJC.c $(DEPS) -DDEBUG

clean:
	rm cyclictestURJC
