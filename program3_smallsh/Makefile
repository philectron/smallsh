CC = gcc
CFLAGS = -std=c11 -lpthread -g -Wall -Wextra

SRC = $(wildcard *.c)
HDR = $(wildcard *.h)
OBJ = $(patsubst %.c, %.o, $(SRC))
EXE = smallsh

%.o: %.c $(HDR)
	$(CC) -c $(CFLAGS) $< -o $@

$(EXE): $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@

.PHONY: clean

clean:
	rm -f *.o $(EXE)
