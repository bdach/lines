CC=$(CROSS_COMPILE)gcc
CFLAGS=-Wall --pedantic --std=gnu99

main:
	$(CC) ${CFLAGS} main.c -o main

.PHONY: clean

clean:
	-rm main
