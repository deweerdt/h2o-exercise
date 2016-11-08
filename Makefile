CC=gcc

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
  SSL_CFLAGS=$(shell pkg-config --cflags libssl)
  SSL_LIBS=$(shell pkg-config --libs libssl)
  CFLAGS += "-DH2O_USE_EPOLL "
  CFLAGS += $(shell PKG_CONFIG_PATH=/usr/local/lib64/pkgconfig/ pkg-config --cflags libh2o-evloop)
  LDFLAGS += $(shell PKG_CONFIG_PATH=/usr/local/lib64/pkgconfig/ pkg-config --libs libh2o-evloop)
  LDFLAGS += "-lpthread"
endif
ifeq ($(UNAME_S),Darwin)
  SSL_CFLAGS=$(shell PKG_CONFIG_PATH="$(shell brew --prefix openssl)/lib/pkgconfig" pkg-config --cflags libssl)
  SSL_LIBS=$(shell PKG_CONFIG_PATH="$(shell brew --prefix openssl)/lib/pkgconfig" pkg-config --libs libssl)
  LDFLAGS += "-luv"
  LDFLAGS += $(shell pkg-config --libs libh2o)
  CFLAGS += $(shell pkg-config --cflags libh2o)
endif

CFLAGS += -Wall -g -O2

all: server server.crt

server.crt:
	openssl req -nodes -subj "/C=US/ST=California/L=San Francisco/O=ACME Inc./OU=Org/CN=localhost" -x509 -newkey rsa:4096 -keyout server.key -out server.crt -days 365

server: server.c
	$(CC) $(CFLAGS) $(SSL_CFLAGS) -o $@ $< $(LDFLAGS) $(SSL_LIBS) -lcrypto
