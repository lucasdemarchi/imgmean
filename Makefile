
CC=g++
CCFLAGS+=-O3 -Wall -c

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
