BIN = turnstile
OBJS = $(BIN).o
CC ?= gcc
LIBS = -lfcgi -lmarkdown
CFLAGS := ${CFLAGS}
CFLAGS += -Wall -std=c99 -fstack-protector-strong $(INC) $(LIBS)
PREFIX ?= /usr/local
BINDIR ?= ${PREFIX}/bin

build: $(BIN)

$(BIN): $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS)

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm -fv *.o $(BIN)
