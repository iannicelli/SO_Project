CC=gcc
CXXOPTS=--std=g++11 -g -Wall 
CXX=g++
CCOPTS=--std=c++17  -Wall 
AR=ar


BINS=my_MMU

HEADERS=chrono.h

LIBS=

%.o:	%.c $(HEADERS)
	$(CC) $(CCOPTS) -c -o $@  $<

%.o:	%.cpp $(HEADERS)
	$(CXX) $(CXXOPTS) -c -o $@  $<

.phony: clean all


all:	$(BINS) $(LIBS)

my_MMU: my_MMU.cpp $(OBJS)
	$(CXX) $(CCOPTS) -o $@ $^

clean:
	rm -rf *.o *~ $(LIBS) $(BINS)
