##
## Sergio Botero
## Makefile dealer
##

SRC = src/sb_*.c
HEA = src/sb_*.h
CC = gcc
CFLAGS = -g -lpthread
RM = rm -f
BIN = dealer

$(BIN): $(SRC) $(HEA)
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	$(RM) $(BIN)