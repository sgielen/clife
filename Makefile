CFLAGS=-lcrypto -O3 -Wall -I/usr/local/include -std=c++11

all: single multi_indep multicolor

single: single.cpp Makefile clife.hpp
	g++ -o single single.cpp $(CFLAGS)

multi_indep: multi_indep.cpp Makefile clife.hpp
	g++ -o multi_indep multi_indep.cpp $(CFLAGS)

multicolor: multicolor.cpp Makefile clife.hpp
	g++ -o multicolor multicolor.cpp $(CFLAGS)
