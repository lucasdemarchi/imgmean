
CC=gcc
CFLAGS+=-O6 -Wall -c -std=c99

SOURCES=imgsum.c parser.c
OBJECTS=$(SOURCES:.c=.o)
INCLUDES=`pkg-config --cflags gtk+-2.0  gdk-pixbuf-2.0`
EXECUTABLE=imgsum
LDFLAGS=`pkg-config --libs gtk+-2.0 gdk-pixbuf-2.0` -pthread


$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) $< -o $@

all: $(SOURCES) $(EXECUTABLE)

clean:
	rm *.o
	rm $(EXECUTABLE)
