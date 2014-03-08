single: single.cpp Makefile clife.hpp
	g++ -o single single.cpp -lcrypto -O3 -Wall

multi_indep: multi_indep.cpp Makefile clife.hpp
	g++ -o multi_indep multi_indep.cpp -lcrypto -O3 -Wall
