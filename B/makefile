CC = gcc
FLAGS = -Wall

all: libproactor.a

libproactor.a : proactor.o proactor.h
	ar -rc libproactor.a proactor.o

.PHONY: clean all

clean:
	rm -f *.o *.a