#pragma once

#include <iostream>
#include <vector>
#include <stdexcept>
#include <stdlib.h>
#include <cassert>

extern "C" {
#include "xxhash/xxhash.c"
}

/** Written by Sjors Gielen, eth0 winter 2014
 *  Feel free to use this for anything you like
 */

/**
 * A ValueType must be copyable and implement:
 * - ValueType() -> unset
 * - ValueType(vector<ValueType>) -> set, init from 8 neighbors, 3 are set
 * - ValueType(bool) -> set, init randomly
 * - operator bool() const -> return if set
 * - char ValueType::getChar() const -> get one character for terminal print
 * - void ValueType::print(ostream&) const -> print whatever you want
 * - std::string hash() const -> return some hash unique to this value
 */

template <typename ValueType>
struct GameOfLifeRow {
	typedef std::vector<ValueType> VectorType;
	typedef typename VectorType::iterator IteratorType;
	
	GameOfLifeRow(int w, IteratorType row_ptr)
	: width(w)
	, row_ptr(row_ptr) {
	}

	ValueType &operator[](unsigned int x) {
		assert(x < width);
		IteratorType copy = row_ptr;
		while(x-- > 0) {
			copy++;
		}
		return *copy;
	}

	ValueType const &operator[](unsigned int x) const {
		assert(x < width);
		IteratorType copy = row_ptr;
		while(x-- > 0) {
			copy++;
		}
		return *copy;
	}

private:
	int width;
	IteratorType row_ptr;
};

template <typename ValueType, bool wrapping = true>
struct GameOfLifeField {
	GameOfLifeField(int w, int h)
	: field(w * h, ValueType())
	, height(h)
	, width(w) {
	}

	void resize(int w, int h) {
		std::vector<ValueType> newfield(w * h, ValueType());
		int minwidth = w < width ? w : width;
		int minheight = h < height ? h : height;
		for(int x = 0; x < minwidth; ++x) {
			for(int y = 0; y < minheight; ++y) {
				newfield[y * w + x] = field[y * width + x];
			}
		}
		height = h;
		width = w;
		field = std::move(newfield);
	}

	inline int get_height() { return height; }
	inline int get_width() { return width; }

	GameOfLifeRow<ValueType> operator[](unsigned int y) {
		assert(y < height);
		return GameOfLifeRow<ValueType>(width, field.begin() + y * width);
	}

	bool is_in_bounds(int y, int x) {
		return y >= 0 && y < height && x >= 0 && x < width;
	}

	bool is_inside_and_set(int y, int x) {
		return is_in_bounds(y, x) && is_set(y, x);
	}

	void add_if_inside_and_set(std::vector<ValueType> &vt, int y, int x) {
		if(is_inside_and_set(y, x)) {
			vt.push_back(field[y * width + x]);
		}
	}

	void add_if_inside_or_wrapping_and_set(std::vector<ValueType> &vt, int y, int x) {
		if(wrapping) {
			y += height;
			y %= height;
			x += width;
			x %= width;
		}
		return add_if_inside_and_set(vt, y, x);
	}

	bool is_set(int y, int x) {
		assert(is_in_bounds(y, x));
		return field[y * width + x];
	}

	void neighbors_set(int y, int x, std::vector<ValueType> &neigh) {
		neigh.resize(0);
		add_if_inside_or_wrapping_and_set(neigh, y-1, x-1);
		add_if_inside_or_wrapping_and_set(neigh, y-1, x  );
		add_if_inside_or_wrapping_and_set(neigh, y-1, x+1);
		add_if_inside_or_wrapping_and_set(neigh, y  , x-1);
		add_if_inside_or_wrapping_and_set(neigh, y  , x+1);
		add_if_inside_or_wrapping_and_set(neigh, y+1, x-1);
		add_if_inside_or_wrapping_and_set(neigh, y+1, x  );
		add_if_inside_or_wrapping_and_set(neigh, y+1, x+1);
	}

	void nextState() {
		std::vector<ValueType> new_field(width * height, ValueType());
		std::vector<ValueType> neighbors;
		neighbors.reserve(8);
		for(int y = 0; y < height; ++y) {
			for(int x = 0; x < width; ++x) {
				neighbors_set(y, x, neighbors);
				int num = neighbors.size();
				bool set = is_set(y, x);
				if(set && (num == 2 || num == 3)) {
					new_field[y * width + x] = field[y * width + x];
					new_field[y * width + x].age_once();
				} else if(num == 3) {
					new_field[y * width + x] = ValueType(neighbors);
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
				os << field[i * width + j].getChar();
			}
			os << "|" << std::endl;
		}
		os << "+";
		for(int j = 0; j < width; ++j) {
			os << "-";
		}
		os << "+" << std::endl << std::endl;
	}

	void print_simple(std::ostream &os) {
		for(int i = 0; i < height; ++i) {
			if(i == 0) {
				field[0].begin_screen(os);
			}

			for(int j = 0; j < width; ++j) {
				if(j == 0) {
					field[i * width].begin_line(os);
				}

				field[i * width + j].print(os);

				if(j == width - 1) {
					field[i * width + j].end_line(os);
				}
			}

			if(i == height - 1) {
				field[i].end_screen(os);
			}
		}
		os << std::flush;
	}

	void set(int x, int y, ValueType value) {
		field[y * width + x] = value;
	}

	void generateHMirror() {
		for(int y = 0; y < height; ++y) {
			for(int x = 0; x < (width + 1) / 2; ++x) {
				ValueType value = ValueType(rand() % 4 < 1);
				set(x, y, value);
				set(width - x - 1, y, value);
			}
		}
	}

	void generateRotated() {
		for(int y = 0; y < height; ++y) {
			for(int x = 0; x < (width + 1) / 2; ++x) {
				ValueType value = ValueType(rand() % 4 < 1);
				set(x, y, value);
				set(width - x - 1, height - y - 1, value);
			}
		}
	}

	uint64_t field_hash() const {
		XXH3_state_t state;
		XXH3_64bits_reset(&state);
		for(auto const &cell : field) {
			bool value = bool(cell);
			XXH3_64bits_update(&state, &value, sizeof(value));
		}
		return XXH3_64bits_digest(&state);
	}

	void generateRandom(int chance_set) {
		if (rand() & 1) {
			generateRotated();
		} else {
			generateHMirror();
		}
	}

private:
	std::vector<ValueType> field;
	int height;
	int width;
};
