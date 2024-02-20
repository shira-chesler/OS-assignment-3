CC = gcc
FLAGS = -Wall
LIBS = -lpthread

all: server client

server: server.o server.h
	$(CC) $(FLAGS) -o server server.o $(LIBS) -lm

client: client.o client.h
	$(CC) $(FLAGS) -o client client.o $(LIBS)

.Phony: clean all

clean:
	rm -f server client *.o