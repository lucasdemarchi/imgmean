
CC=gcc
LD=g++
CFLAGS = -std=gnu99
SOURCES=src/imgmean.c src/parse_options.c
OBJECTS=$(SOURCES:.c=.o)
INCLUDES=
EXECUTABLE=imgmean

LDFLAGS= -lfreeimage -pthread

# Compile FreeImage statically?
static_freeimage := false

# Optimization
optimize := true

# processor optmization of GCC. Look for -march
# If you are not sure of your system, just set 'specialize' to false. if
# it is a different one look for the -march option param of gcc and
# write your platforms name optimize for pentium4 ? / disabled if
# optimize is false
specialize := true
arch := core2

# sse flags
usesse := true

include Makefile.flags


ifeq ($(static_freeimage),true)
  LDFLAGS += -static
else
  LDFLAGS += -dynamic
endif

$(EXECUTABLE): $(OBJECTS)
	$(LD)  $(OBJECTS) -o $@ $(LDFLAGS)

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

all: $(SOURCES) $(EXECUTABLE)

clean:
	rm -f src/*.o
	rm -f $(EXECUTABLE)
