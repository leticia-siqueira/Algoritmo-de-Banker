CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -pedantic

banker: banker.c
	$(CC) $(CFLAGS) -o banker banker.c

clean:
	rm -f banker

.PHONY: clean