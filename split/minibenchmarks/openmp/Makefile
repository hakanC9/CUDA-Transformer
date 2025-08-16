CC=gcc
CFLAGS=-fopenmp -std=c99 -O3 -DNDEBUG -Wall
LDFLAGS=-fopenmp
LDLIBS=-lm
ARTIFACTS=integrate heat fft


all: $(ARTIFACTS)

%: %.c

clean:
	rm -f $(ARTIFACTS) *.o
