OBJECTS=messages_client.o \
	erpc_setup_mbf_dynamic.o \
	setup.o \
	/usr/local/lib/liberpc.a


CXXFLAGS=-I/usr/local/include/erpc/ -std=c++14
LDFLAGS=-lpthread
CC=g++

all: leds calc

leds: $(OBJECTS) leds.o
calc: $(OBJECTS) calc.o



clean:
	rm -f *.o leds calc
