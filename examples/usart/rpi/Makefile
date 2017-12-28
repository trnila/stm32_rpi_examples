OBJECTS=messages_client.o \
	main.o \
	erpc_setup_mbf_dynamic.o

CXXFLAGS=-I/usr/local/include/erpc/ -std=c++14
LDFLAGS=-lpthread
CC=g++
TARGET=main

$(TARGET): $(OBJECTS) /usr/local/lib/liberpc.a


clean:
	rm -f *.o $(TARGET)
