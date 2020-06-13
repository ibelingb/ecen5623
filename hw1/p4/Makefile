INCLUDE_DIRS = 
LIB_DIRS = 
CC=gcc

CDEFS=
CFLAGS= -O0 $(INCLUDE_DIRS) $(CDEFS)
LIBS= 

HFILES= 
CFILES= p4_fibTest.c

SRCS= ${HFILES} ${CFILES}
OBJS= ${CFILES:.c=.o}

all:	p4_fibTest

clean:
	-rm -f *.o *.d
	-rm -f perfmon p4_fibTest

distclean:
	-rm -f *.o *.d
	-rm -f p4_fibTest

p4_fibTest: p4_fibTest.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $@.o -lpthread

depend:

.c.o:
	$(CC) $(CFLAGS) -c $<