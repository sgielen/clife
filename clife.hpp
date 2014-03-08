#pragma once

#include <iostream>
#include <vector>
#include <stdexcept>
#include <openssl/md5.h>
#include <stdlib.h>

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

template <typename ValueType, int width>
struct GameOfLifeRow {
	typedef std::vector<ValueType> VectorType;
	typedef typename VectorType::iterator IteratorType;
	
	GameOfLifeRow(IteratorType row_ptr)
	: row_ptr(row_ptr) {
	}

	ValueType &operator[](unsigned int x) {
		if(x >= width) {
			throw std::runtime_error("Out of bounds");
		}
		IteratorType copy = row_ptr;
		while(x-- > 0) {
			copy++;
		}
		return *copy;
	}

private:
	IteratorType row_ptr;
};

template <typename ValueType, int height, int width, bool wrapping = true>
struct GameOfLifeField {
	GameOfLifeField() : field(width * height, ValueType()) {
	}

	GameOfLifeRow<ValueType, width> operator[](unsigned int y) {
		if(y >= height) {
			throw std::runtime_error("Out of bounds");
		}
		return GameOfLifeRow<ValueType, width>(field.data() + y * width);
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
		if(!is_in_bounds(y, x)) {
			throw std::runtime_error("Out of bounds");
		}
		return field[y * width + x];
	}

	std::vector<ValueType> neighbors_set(int y, int x) {
		std::vector<ValueType> neigh;
		add_if_inside_or_wrapping_and_set(neigh, y-1, x-1);
		add_if_inside_or_wrapping_and_set(neigh, y-1, x  );
		add_if_inside_or_wrapping_and_set(neigh, y-1, x+1);
		add_if_inside_or_wrapping_and_set(neigh, y  , x-1);
		add_if_inside_or_wrapping_and_set(neigh, y  , x+1);
		add_if_inside_or_wrapping_and_set(neigh, y+1, x-1);
		add_if_inside_or_wrapping_and_set(neigh, y+1, x  );
		add_if_inside_or_wrapping_and_set(neigh, y+1, x+1);
		return neigh;
	}

	void nextState() {
		std::vector<ValueType> new_field(width * height, ValueType());
		for(int y = 0; y < height; ++y) {
			for(int x = 0; x < width; ++x) {
				std::vector<ValueType> neighbors = neighbors_set(y, x);
				int num = neighbors.size();
				bool set = is_set(y, x);
				if(set && (num == 2 || num == 3)) {
					new_field[y * width + x] = field[y * width + x];
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
			for(int j = 0; j < width; ++j) {
				field[i * width + j].print(os);
			}
		}
		os << std::flush;
	}

	std::string field_hash() const {
		char digest[MD5_DIGEST_LENGTH];
		typename std::vector<ValueType>::const_iterator it;
		std::string str_field;
		for(it = field.begin(); it != field.end(); ++it) {
			str_field += it->hash();
		}
		MD5((unsigned char*) str_field.c_str(), str_field.length(), (unsigned char*)&digest);
		return std::string(&*digest, MD5_DIGEST_LENGTH);
	}

	void generateRandom(int chance_set) {
		if(chance_set < 0 || chance_set > 100) {
			throw std::runtime_error("Out of bounds");
		}
		for(int i = 0; i < height; ++i) {
			for(int j = 0; j < width; ++j) {
				field[i * width + j] = ValueType(rand() % 100 < chance_set ? true : false);
			}
		}
	}

private:
	std::vector<ValueType> field;
};
