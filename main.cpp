#include <iostream>
#include <vector>
#include <sstream>
#include "clife.hpp"
#include <unistd.h>
#include <openssl/md5.h>
#include <stdlib.h>
#include <time.h>

/** Written by Sjors Gielen, eth0 winter 2014
 *  Feel free to use this for anything you like
 */

template <int height, int width>
void print_rgb(GameOfLifeField<height, width> red, GameOfLifeField<height, width> green, GameOfLifeField<height, width> blue) {
	char on = 0x80;
	char off = 0x00;
	for(int y = 0; y < height; ++y) {
		for(int x = 0; x < width; ++x) {
			std::cout << (red.is_set(y, x) ? on : off);
			std::cout << (green.is_set(y, x) ? on : off);
			std::cout << (blue.is_set(y, x) ? on : off);
		}
	}
	std::cout << std::flush;
}

template <typename FieldType>
void check_stop_condition(FieldType field, std::vector<std::string> &earlier_hashes, bool &done, int &repeats_to_do) {
	std::string hash = field.field_hash();
	for(int i = 0; i < earlier_hashes.size(); ++i) {
		if(earlier_hashes[i] == hash) {
			done = true;
			repeats_to_do = 10;
			break;
		}
	}
	earlier_hashes.push_back(hash);
}

int main(int argc, char *argv[]) {
	int microsleeptime = 100000;
	if(argc == 2) {
		std::stringstream ss;
		ss << argv[1];
		ss >> microsleeptime;
		if(!ss) {
			std::cerr << "Usage: " << argv[0] << " [microsleeptime]" << std::endl;
		}
	} else if(argc > 2) {
		std::cerr << "Usage: " << argv[0] << " [microsleeptime]" << std::endl;
	}

	srand(time(NULL));
	std::vector<std::string> earlier_hashes;
	GameOfLifeField<8, 80> red;
	GameOfLifeField<8, 80> green;
	GameOfLifeField<8, 80> blue;

	red.generateRandom();
	green.generateRandom();
	blue.generateRandom();

	bool red_done = false;
	bool green_done = false;
	bool blue_done = false;
	int repeats_to_do = 0;
	print_rgb(red, green, blue);
	while(!red_done || !green_done || !blue_done || repeats_to_do > 0) {
		if(repeats_to_do > 0) {
			--repeats_to_do;
		}
		red.nextState();
		green.nextState();
		blue.nextState();
		print_rgb(red, green, blue);
		if(!red_done || !green_done || !blue_done) {
			check_stop_condition(red, earlier_hashes, red_done, repeats_to_do);
			check_stop_condition(green, earlier_hashes, green_done, repeats_to_do);
			check_stop_condition(blue, earlier_hashes, blue_done, repeats_to_do);
		}
		usleep(microsleeptime);
		continue;
	}
}
