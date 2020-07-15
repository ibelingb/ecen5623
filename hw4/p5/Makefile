INCLUDE_DIRS = 
LIB_DIRS = 
CC=gcc

CDEFS=
CFLAGS= -O0 -g $(INCLUDE_DIRS) $(CDEFS)
LIBS= -lrt -lpthread

HFILES= 
CFILES= transform_compare.c

SRCS= ${HFILES} ${CFILES}
OBJS= ${CFILES:.c=.o}

all:	transform_compare

clean:
	-rm -f *.o *.d
	-rm -f transform_compare

distclean:
	-rm -f *.o *.d

transform_compare: ${OBJS}
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $@.o $(LIBS)

depend:

.c.o:
	$(CC) $(CFLAGS) -c $<