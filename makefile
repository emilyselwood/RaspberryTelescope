
PROG=raspberrytelescope
CC=gcc
CFLAGS=-O2 -Wall -g -rdynamic -std=c99
LDFLAGS=-ldl -pthread -lgphoto2 -lconfig -lm

raspberrytelescope:
	$(CC) stringutils.c fileutils.c telescopecamera.c timelapse.c mongoose.c webserver.c $(CFLAGS) $(LDFLAGS) -o $(PROG)

.PHONY : clean
clean:
	rm -f *.o $(PROG)
