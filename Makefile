
CC=g++
CCFLAGS+=-O6 -Wall -Dcimg_display=0 -c

SOURCES=imgsum.cc
OBJECTS=$(SOURCES:.cc=.o)
EXECUTABLE=imgsum
LDFLAGS=


$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cc.o:
	$(CC) $(CCFLAGS) $< -o $@

all: $(SOURCES) $(EXECUTABLE)

clean:
	rm $(OBJECTS) $(EXECUTABLE)
