CC=clang++
CFLAGS=-c -Wall
LDFLAGS=-I ../boost_1_53_0 -I ../boost_1_53_0/boost/gil/extension/numeric
LFLAGS=-ljpeg
SOURCES=main.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=main

all: $(SOURCES) $(EXECUTABLE) clean

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@ $(LFLAGS)

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(OBJECTS) *.o *~
