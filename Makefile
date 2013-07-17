CFLAGS+=-Wall -g -lownet

all:  ifrit

ifrit: main.c statsd.o
	$(CC) $(CFLAGS) -o ifrit statsd.o main.c

statsd.o: statsd.h statsd.c

clean:
	rm -f *.o ifrit 
