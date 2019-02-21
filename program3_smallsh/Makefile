CC = gcc
CFLAGS = -std=c11 -lpthread -I. -g -Wall -Wextra

ALL = smallsh

all: $(ALL)

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

smallsh: smallsh.c
	$(CC) $(CFLAGS) $? -o $@

.PHONY: clean

clean:
	rm -f *.o $(ALL)
