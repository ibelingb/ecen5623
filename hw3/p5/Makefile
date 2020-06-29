INCLUDE_DIRS = 
LIB_DIRS = 
CC=gcc

CDEFS=
CFLAGS= -O0 $(INCLUDE_DIRS) $(CDEFS)
LIBS= 

HFILES= 
CFILES= p5_mutexDataTimeout.c

SRCS= ${HFILES} ${CFILES}
OBJS= ${CFILES:.c=.o}

all:	p5_mutexDataTimeout

clean:
	-rm -f *.o *.d
	-rm -f perfmon p5_mutexDataTimeout

distclean:
	-rm -f *.o *.d
	-rm -f p5_mutexDataTimeout

p5_mutexDataTimeout: p5_mutexDataTimeout.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $@.o -lpthread

depend:

.c.o:
	$(CC) $(CFLAGS) -c $<