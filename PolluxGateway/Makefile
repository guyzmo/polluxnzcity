CXX=/usr/bin/g++
RM=/bin/rm -rf
HEADERS=include/
SRC=src/

CFLAGS=-Wno-write-strings

%.o:
	$(CXX) $(CFLAGS) -I$(HEADERS) -o $*.o -c $*.C

pollux_gateway: $(SRC)/pollux_gateway.C src/beagle_serial.o src/xbee_communicator.o
	$(CXX) $(CFLAGS) -I$(HEADERS) -o $@ $^

all: pollux_gateway

clean:
	$(RM) src/*.o pollux_gateway
