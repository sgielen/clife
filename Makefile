main: main.cpp Makefile clife.hpp
	g++ -o main main.cpp -lcrypto -O3 -Wall
