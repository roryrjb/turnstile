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
	rm -v $(BIN)

dependencies:
	sudo apt install -y libfcgi-dev libmarkdown2-dev

deb: build
	mkdir -p output
	mkdir -p ddb/usr/local/bin/
	cp -v $(BIN) ddb/usr/local/bin/
	ddb -t ddb -o output

.PHONY: clean dependencies deb
