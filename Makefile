
CC=gcc
CFLAGS = -std=gnu99
SOURCES=imgsum.c parse_options.c
OBJECTS=$(SOURCES:.c=.o)
INCLUDES=`pkg-config --cflags gtk+-2.0  gdk-pixbuf-2.0`
EXECUTABLE=imgsum

LDFLAGS=`pkg-config --libs gtk+-2.0 gdk-pixbuf-2.0` -pthread


# Optimization
optimize := false

# processor optmization of GCC. Look for -march
# If you are not sure of your system, just set 'specialize' to false. if
# it is a different one look for the -march option param of gcc and
# write your platforms name optimize for pentium4 ? / disabled if
# optimize is false
specialize := true
arch := core2

# sse flags
usesse := false

include Makefile.flags


$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

all: $(SOURCES) $(EXECUTABLE)

clean:
	rm *.o
	rm $(EXECUTABLE)
