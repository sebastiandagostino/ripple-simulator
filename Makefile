DEFS       = 
LIBS       = $(wildcard lib/*.hpp)
DEBUGFLAGS = -DDEBUG -g
CXXFLAGS   = -O3 -std=c++11 -Wall -Wno-sign-compare -Wno-char-subscripts \
             -Wno-invalid-offsetof -Wformat $(DEBUGFLAGS) $(DEFS)
HEADERS    = $(wildcard *.h)
OBJECTS    = $(patsubst %.cpp, %.o, $(wildcard src/*.cpp))

.PHONY: default all clean

default: clean gen sim

all: default

%.o: %.cpp $(HEADERS)
    $(CXX) -c $(CXXFLAGS) -o $@ $<

.PRECIOUS: $(TARGET) $(OBJECTS)

sim: $(OBJECTS) SimRun.cpp
    $(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

gen: $(OBJECTS) SimGen.cpp
    $(CXX) $(CXXFLAGS) -o $@ $^

clean:
    -rm -f *.o
    -rm -f src/*.o
    -rm -f sim
    -rm -f gen
