CXX=/usr/bin/g++
RM=/bin/rm -rf

CFLAGS=-Wno-write-strings

.o:
	$(CXX) $(CFLAGS) -c -o $@

pollux_gateway: pollux_gateway.C beagle_serial.o xbee_communicator.o
	$(CXX) $(CFLAGS) -o $@ $^

all: pollux_gateway

clean:
	$(RM) *.o pollux_gateway
