#include <iostream>
#include <vector>
#include <sstream>
#include "clife.hpp"
#include "util.hpp"
#include <unistd.h>
#include <openssl/md5.h>
#include <stdlib.h>
#include <time.h>

/** Written by Sjors Gielen, eth0 winter 2014
 *  Feel free to use this for anything you like
 */

template <char ifSet, char ifUnset>
struct SimpleValue {
	bool value;

	SimpleValue() : value(false) {}
	SimpleValue(std::vector<SimpleValue> vec) : value(true) {}
	SimpleValue(bool value) : value(value) {}

	operator bool() const { return value; }
	std::string hash() const {
		return (value ? "1" : "0");
	}

	char getChar() const {
		return (value ? 'o' : ' ');
	}
	void print(std::ostream &os) const {
		os << (value ? ifSet : ifUnset);
	}

	void age_once() const {/* ignore */}
};

template <typename FieldType>
void check_stop_condition(FieldType field, std::vector<std::string> &earlier_hashes, bool &done, int &repeats_to_do) {
	std::string hash = field.field_hash();
	for(size_t i = 0; i < earlier_hashes.size(); ++i) {
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

	init_random();

	std::vector<std::string> earlier_hashes;
	GameOfLifeField<SimpleValue<char(0x80), 0>> field(8, 80);

	field.generateRandom(35);

	bool field_done = false;
	int repeats_to_do = 0;
	field.print(std::cout);
	while(!field_done || repeats_to_do > 0) {
		if(repeats_to_do > 0) {
			--repeats_to_do;
		}
		field.nextState();
		field.print(std::cout);
		if(!field_done) {
			check_stop_condition(field, earlier_hashes, field_done, repeats_to_do);
		}
		usleep(microsleeptime);
		continue;
	}
}
