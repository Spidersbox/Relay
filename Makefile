all: relay

CFLAGS = -O2 -Wall

relay: relay.c
	${CC} -DUNIT_TEST -o $@ $^ -lhidapi-libusb

clean:
	rm -f relay *.o

