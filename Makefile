CC=gcc

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
  SSL_CFLAGS=$(shell pkg-config --cflags libssl)
  SSL_LIBS=$(shell pkg-config --libs libssl)
endif
ifeq ($(UNAME_S),Darwin)
  SSL_CFLAGS=$(shell PKG_CONFIG_PATH="$(shell brew --prefix openssl)/lib/pkgconfig" pkg-config --cflags libssl)
  SSL_LIBS=$(shell PKG_CONFIG_PATH="$(shell brew --prefix openssl)/lib/pkgconfig" pkg-config --libs libssl)
endif

CFLAGS += -Wall -g -O2

all: server server.crt

server.crt:
	openssl req -nodes -subj "/C=US/ST=Oregon/L=Portland/O=Company Name/OU=Org/CN=www.example.com" -x509 -newkey rsa:4096 -keyout server.key -out server.crt -days 365

server: server.c
	$(CC) $(CFLAGS) $(shell pkg-config --cflags libh2o) $(SSL_CFLAGS) -o $@ $< $(shell pkg-config --libs libh2o) $(SSL_LIBS) -luv -lcrypto
