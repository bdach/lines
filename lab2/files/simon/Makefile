CC=$(CROSS_COMPILE)gcc
CFLAGS=-Wall --pedantic --std=gnu99 -g

simon:
	$(CC) ${CFLAGS} main.c -o simon

.PHONY: clean

clean:
	-rm simon
