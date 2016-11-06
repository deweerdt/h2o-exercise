CC=gcc
CFLAGS=-Wall -g -O2
SSL_CFLAGS=$(shell PKG_CONFIG_PATH="$(shell brew --prefix openssl)/lib/pkgconfig" pkg-config --cflags libssl)
SSL_LIBS=$(shell PKG_CONFIG_PATH="$(shell brew --prefix openssl)/lib/pkgconfig" pkg-config --libs libssl)

server: server.c
	$(CC) $(CFLAGS) $(shell pkg-config --cflags libh2o) $(SSL_CFLAGS) -o $@ $< $(shell pkg-config --libs libh2o) $(SSL_LIBS) -luv -lcrypto
