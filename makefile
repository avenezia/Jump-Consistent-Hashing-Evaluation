CC=g++
CFLAGS=-c -Wall -Werror
CSTD=-std=c++11
LIBS=-lboost_program_options

all: jumpConsistentHashing.o
	$(CC) $(CSTD) -o jumpConsistentHashing jumpConsistentHashing.o $(LIBS)

jumpConsistentHashing.o: jumpConsistentHashing.cpp
	$(CC) $(CFLAGS) $(CSTD) jumpConsistentHashing.cpp

clean:
	rm -rf *.o jumpConsistentHashing
