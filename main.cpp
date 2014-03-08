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
	GameOfLifeField<8, 80> field;

	std::string set("X");
	std::string unset(" ");

	field.generateRandom();
	field.print_simple(std::cout, set, unset);
	bool done = false;
	int repeats_to_do = 0;
	while(!done || repeats_to_do > 0) {
		if(repeats_to_do > 0) {
			--repeats_to_do;
		}
		field.nextState();
		field.print_simple(std::cout, set, unset);
		if(!done) {
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
		usleep(microsleeptime);
		continue;
	}
}
