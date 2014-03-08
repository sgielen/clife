#include <iostream>
#include <vector>
#include <stdexcept>
#include <sstream>
#include <unistd.h>
#include <openssl/md5.h>
#include <stdlib.h>
#include <time.h>

/** Written by Sjors Gielen, eth0 winter 2014
 *  Feel free to use this for anything you like
 */

template <int width>
struct GameOfLifeRow {
	GameOfLifeRow(char *row_ptr)
	: row_ptr(row_ptr) {
	}

	char &operator[](unsigned int x) {
		if(x >= width) {
			throw std::runtime_error("Out of bounds");
		}
		return *(row_ptr + x);
	}

private:
	char *row_ptr;
};

template <int height, int width>
struct GameOfLifeField {
	GameOfLifeField() : field(width * height, 0) {
	}

	GameOfLifeRow<width> operator[](unsigned int y) {
		if(y >= height) {
			throw std::runtime_error("Out of bounds");
		}
		return GameOfLifeRow<width>(&field[y * width]);
	}

	bool is_in_bounds(int y, int x) {
		return y >= 0 && y < height && x >= 0 && x < width;
	}

	bool is_inside_and_set(int y, int x) {
		return is_in_bounds(y, x) && is_set(y, x);
	}

	bool is_set(int y, int x) {
		if(!is_in_bounds(y, x)) {
			throw std::runtime_error("Out of bounds");
		}
		return field[y * width + x] == 1;
	}

	int neighbors_set(int y, int x) {
		int num_neighbors = 0;
		if(is_inside_and_set(y-1, x-1)) num_neighbors++;
		if(is_inside_and_set(y-1, x  )) num_neighbors++;
		if(is_inside_and_set(y-1, x+1)) num_neighbors++;
		if(is_inside_and_set(y  , x-1)) num_neighbors++;
		if(is_inside_and_set(y  , x+1)) num_neighbors++;
		if(is_inside_and_set(y+1, x-1)) num_neighbors++;
		if(is_inside_and_set(y+1, x  )) num_neighbors++;
		if(is_inside_and_set(y+1, x+1)) num_neighbors++;
		return num_neighbors;
	}

	void nextState() {
		std::vector<char> new_field(width * height, 0);
		for(int y = 0; y < height; ++y) {
			for(int x = 0; x < width; ++x) {
				int neighbors = neighbors_set(y, x);
				bool set = is_set(y, x);
				if((set && neighbors == 2) || neighbors == 3) {
					new_field[y * width + x] = 1;
				}
			}
		}
		field = new_field;
	}

	void print(std::ostream &os) {
		os << "+";
		for(int j = 0; j < width; ++j) {
			os << "-";
		}
		os << "+" << std::endl;
		for(int i = 0; i < height; ++i) {
			os << "|";
			for(int j = 0; j < width; ++j) {
				if(field[i * width + j]) {
					os << "o";
				} else {
					os << " ";
				}
			}
			os << "|" << std::endl;
		}
		os << "+";
		for(int j = 0; j < width; ++j) {
			os << "-";
		}
		os << "+" << std::endl << std::endl;
	}

	void print_simple(std::ostream &os, std::string set, std::string unset) {
		for(int i = 0; i < height; ++i) {
			for(int j = 0; j < width; ++j) {
				os << (field[i * width + j] ? set : unset);
			}
		}
		os << std::flush;
	}

	std::string field_hash() const {
		char digest[MD5_DIGEST_LENGTH];
		std::string str_field(field.data(), field.size());
		MD5((unsigned char*) str_field.c_str(), field.size(), (unsigned char*)&digest);
		return std::string(&*digest, MD5_DIGEST_LENGTH);
	}

	void generateRandom() {
		int chance_set = 15; // percentage
		for(int i = 0; i < height; ++i) {
			for(int j = 0; j < width; ++j) {
				field[i * width + j] = rand() % 100 < chance_set ? 1 : 0;
			}
		}
	}

private:
	std::vector<char> field;
};

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
