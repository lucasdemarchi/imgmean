
CC=g++
CCFLAGS+=-O6 -Wall -c

SOURCES=imgsum.cc
OBJECTS=$(SOURCES:.cc=.o)
INCLUDES=`pkg-config --cflags gtk+-2.0  gdk-pixbuf-2.0`
EXECUTABLE=imgsum
LDFLAGS=`pkg-config --libs gtk+-2.0 gdk-pixbuf-2.0`


$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cc.o:
	$(CC) $(CCFLAGS) $(INCLUDES) $< -o $@

all: $(SOURCES) $(EXECUTABLE)

clean:
	rm $(OBJECTS) $(EXECUTABLE)
