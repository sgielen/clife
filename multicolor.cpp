#include <iostream>
#include <vector>
#include <sstream>
#include "clife.hpp"
#include "util.hpp"
#include <pthread.h>
#include <unistd.h>
#include <openssl/md5.h>
#include <stdlib.h>
#include <time.h>
#include <cassert>
#include <cmath>
#include <ctime>
#include <algorithm>
#include <functional>
#include <sys/ioctl.h>

/** Written by Sjors Gielen, eth0 winter 2014
 *  Feel free to use this for anything you like
 */

constexpr bool to_ledscreen = false;
constexpr bool concise = true;
std::function<int()> get_width;

// Uncomment to get terminal-based size
#define FIXED_SIZE
#define FIXED_WIDTH 80
#define FIXED_HEIGHT 8

struct MulticolorValue {
	bool value;
	uint8_t red;
	uint8_t green;
	uint8_t blue;
	uint8_t age;

	MulticolorValue() : value(false), red(0), green(0), blue(0), age(0){}
	MulticolorValue(std::vector<MulticolorValue> vec) : value(true), age(0) {
		std::vector<int> hues;
		for(auto const &cell : vec) {
			if(cell.value) {
				float r = float(cell.red) / UINT8_MAX;
				float g = float(cell.green) / UINT8_MAX;
				float b = float(cell.blue) / UINT8_MAX;
				float min = std::min(r, std::min(g, b));
				float max = std::max(r, std::max(g, b));
				// hue from 0 to 360
				int hue = max == 0 ? 0 :
				            r == max ? (60 * (0 + (g-b)/(max-min))) :
				            g == max ? (60 * (2 + (b-r)/(max-min))) :
				                       (60 * (4 + (r-g)/(max-min)));
				if(hue < 0) {
					hue += 360;
				}
				hues.push_back(hue);
			}
		}
		assert(hues.size() == 3);
		std::sort(hues.begin(), hues.end());
		int range1 = hues[2] - hues[0];
		int range2 = (hues[0] + 360) - hues[1];
		int range3 = (hues[1] + 360) - hues[2];
		int avghue;
		if(std::min(range1, std::min(range2, range3)) == range1) {
			avghue = (hues[0] + hues[1] + hues[2]) / 3;
		} else if(std::min(range1, std::min(range2, range3)) == range2) {
			avghue = (hues[0] + hues[1] + hues[2] + 360) / 3;
		} else {
			avghue = (hues[0] + hues[1] + hues[2] + 720) / 3;
		}
		avghue %= 360;
		float sat = 1;
		float value = 1;
		int h_i = std::floor(avghue / 60);
		float f = (avghue / 60.) - h_i;
		float p = value * (1 - sat);
		float q = value * (1 - f * sat);
		float t = value * (1 - (1 - f) * sat);

		value *= 255;
		p *= 255;
		q *= 255;
		t *= 255;

		switch(h_i) {
		case 0:
			red = value;
			green = t;
			blue = p;
			break;
		case 1:
			red = q;
			green = value;
			blue = p;
			break;
		case 2:
			red = p;
			green = value;
			blue = t;
			break;
		case 3:
			red = p;
			green = q;
			blue = value;
			break;
		case 4:
			red = t;
			green = p;
			blue = value;
			break;
		case 5:
			red = value;
			green = p;
			blue = q;
			break;
		default:
			abort();
		};
	}
	MulticolorValue(bool value) : value(value), red(0), green(0), blue(0), age(0) {
		if(value) {
			char color = (rand() % 6) + 1;
			red   = (color & 1) ? 255 : 0;
			green = (color & 2) ? 255 : 0;
			blue  = (color & 4) ? 255 : 0;
		}
	}

	void age_once() {
		age = (age + 1) % 50;
	}

	operator bool() const { return value; }
	std::string hash() const {
		if(value) {
			return std::string("1") + char(red) + char(green) + char(blue);
		} else {
			return "0\x00\x00\x00";
		}
	}

	char getChar() const {
		return (value ? 'o' : ' ');
	}

	void begin_screen(std::ostream &os) const {
		if(!to_ledscreen && !concise) {
			std::cout << "+";
			for(int i = 0; i < get_width(); ++i) {
				std::cout << "-";
			}
			std::cout << "+\n";
		}
	}
	void end_screen(std::ostream &os) const {
		begin_screen(os);
		std::cout << "\x1b[1;1H" << std::flush;
	}
	void begin_line(std::ostream &os) const {
		if(!to_ledscreen && !concise) {
			std::cout << "|";
		}
	}
	void end_line(std::ostream &os) const {
		if(!to_ledscreen && !concise) {
			std::cout << "|\n";
		}
	}

	void print(std::ostream &os) const {
		if(value && to_ledscreen) {
			os << red << green << blue;
		} else if(value) {
			double brightness = std::max(0.3, 1 - age / 20.);

			uint8_t redval = (red * brightness) / 43;
			uint8_t greenval = (green * brightness) / 43;
			uint8_t blueval = (blue * brightness) / 43;
			uint8_t code = 16 + 36 * redval + 6 * greenval + blueval;
			os << "\x1b[38;5;" << int(code) << "mo\x1b[m";
		} else if(to_ledscreen) {
			os << '\x00' << '\x00' << '\x00';
		} else {
			os << ' ';
		}
	}
};

template <typename FieldType>
void check_stop_condition(FieldType field, std::vector<std::string> &earlier_hashes, bool &done, int &repeats_to_do) {
	std::string hash = field.field_hash();
	for(size_t i = 0; i < earlier_hashes.size(); ++i) {
		if(earlier_hashes[i] == hash) {
			done = true;
			repeats_to_do = 50;
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

	winsize current_winsize;
#if defined(FIXED_SIZE)
	current_winsize.ws_col = FIXED_WIDTH;
	current_winsize.ws_row = FIXED_HEIGHT;
#else
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &current_winsize);
#endif
	std::vector<std::string> earlier_hashes;
	GameOfLifeField<MulticolorValue> field(current_winsize.ws_col, current_winsize.ws_row);
	get_width = [&field]() {
		return field.get_width();
	};

	field.generateRandom(35);

	bool field_done = false;
	int repeats_to_do = 0;

	pthread_mutex_t mtx;
	pthread_mutex_init(&mtx, nullptr);
	pthread_cond_t cond;
	pthread_cond_init(&cond, nullptr);
	struct timespec ts = {};
	ts.tv_sec = std::time(nullptr);
	pthread_mutex_lock(&mtx);

	while(!field_done || repeats_to_do > 0) {
#if !defined(FIXED_SIZE)
		winsize new_winsize;
		ioctl(STDOUT_FILENO, TIOCGWINSZ, &new_winsize);
		if(current_winsize.ws_col != new_winsize.ws_col
		|| current_winsize.ws_row != new_winsize.ws_row) {
			field.resize(new_winsize.ws_col, new_winsize.ws_row);
		}
		current_winsize = new_winsize;
#endif

		if(repeats_to_do > 0) {
			--repeats_to_do;
		}
		field.nextState();
		field.print_simple(std::cout);
		if(!field_done) {
			check_stop_condition(field, earlier_hashes, field_done, repeats_to_do);
		}
		ts.tv_nsec += 100000000;
		ts.tv_sec += ts.tv_nsec / 1000000000;
		ts.tv_nsec %= 1000000000;
		pthread_cond_timedwait(&cond, &mtx, &ts);
	}

	pthread_mutex_unlock(&mtx);
	pthread_cond_destroy(&cond);
	pthread_mutex_destroy(&mtx);
}
