CC = gcc
CFLAGS = -Wall
LIBS = -lproactor -lpthread -lm
LIB_DIR = ../B
SERVER_OBJS = server.o

all: server

server: server.o 
	$(CC) $(CFLAGS) -o server server.o -L$(LIB_DIR) $(LIBS)

server.o: server.c server.h
	$(CC) $(CFLAGS) -c $< -I$(LIB_DIR) -o $@

.PHONY: clean all

clean:
	rm -f *.o server