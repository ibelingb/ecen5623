INCLUDE_DIRS = 
LIB_DIRS = 
CC=gcc

CDEFS=
CFLAGS= -O0 $(INCLUDE_DIRS) $(CDEFS)
LIBS= 

HFILES= 
CFILES= p2_mutexThreadSync.c

SRCS= ${HFILES} ${CFILES}
OBJS= ${CFILES:.c=.o}

all:	p2_mutexThreadSync

clean:
	-rm -f *.o *.d
	-rm -f perfmon p2_mutexThreadSync

distclean:
	-rm -f *.o *.d
	-rm -f p2_mutexThreadSync

p2_mutexThreadSync: p2_mutexThreadSync.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $@.o -lpthread

depend:

.c.o:
	$(CC) $(CFLAGS) -c $<