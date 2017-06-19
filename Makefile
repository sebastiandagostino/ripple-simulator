# Copyright (c) 2009-2010 Satoshi Nakamoto
# Distributed under the MIT/X11 software license, see the accompanying
# file license.txt or http://www.opensource.org/licenses/mit-license.php.

TARGET     = sim
DEFS       =
LIBS       = -pthread
DEBUGFLAGS = -DDEBUG -g
CXXFLAGS   = -O3 -std=c++11 -Wall -Wno-sign-compare -Wno-char-subscripts \
             -Wno-invalid-offsetof -Wformat $(DEBUGFLAGS) $(DEFS)
HEADERS    = $(wildcard *.h)
OBJECTS    = $(patsubst %.cpp, %.o, $(wildcard *.cpp))

.PHONY: default all clean

default: $(TARGET)

all: default

%.o: %.cpp $(HEADERS)
	$(CXX) -c $(CXXFLAGS) -o $@ $<

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

clean:
	-rm -f *.o
	-rm -f $(TARGET)
