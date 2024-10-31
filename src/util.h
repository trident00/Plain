#pragma once
#include <vector>
#ifdef _DEBUG
#include <iostream>
#endif
#include <stdexcept>
#include <string>

const size_t ARENA_SIZE = 16 * 1024;  // 16 KB arena size


struct Arena
{
	char *memory;
	size_t capacity;
	size_t num_allocs = 0;
	size_t offset = 0;

	Arena(size_t size)
		: capacity(size) {
		memory = (char *)malloc(size);
		if (!memory) throw std::bad_alloc();
	}

	template<typename T>
	T *allocate(size_t size) {
		if (offset + size > capacity) {
			throw std::bad_alloc();
		}
		T *result = reinterpret_cast<T *>(memory + offset);
		offset += size;
		num_allocs++;
		return result;
	}

	void reset() {
		offset = 0;
		num_allocs = 0;
	}

	void free() {
		reset();
		std::free(memory);
		capacity = 0;
	}

};

template<typename T>
struct Array
{
	Array(Arena &arena, size_t cap)
		: arena(arena), capacity(cap), size(0) {
		data = (T *)arena.allocate<T>(sizeof(T) * capacity);
	}

	Arena &arena;
	size_t capacity;
	size_t size;
	T *data;

	void push(const T &val) {
		if (size == capacity) {
			throw std::bad_alloc();
		}
		data[size++] = val;

	}

	T &peek() {
		return data[size - 1];
	}

	const T &peek() const {
		return data[size-1];
	}

	T &operator[](size_t index) {
		if (index >= capacity) {
			throw std::out_of_range("Index out of bounds");
		}
		return data[index];
	}

	const T &operator[](size_t index) const {
		if (index >= capacity) {
			throw std::out_of_range("Index out of bounds");
		}
		return data[index];
	}
};

template<typename T>
struct MutableArray
{

	MutableArray(Arena &arena, size_t init_cap = 16)
		: arena(arena), capacity(init_cap), size(0) {
		data = (T *)arena.allocate<T>(capacity * sizeof(T));
	}

	Arena &arena;
	size_t capacity;
	size_t size;
	T *data;

	void push(const T &val) {
		if (size == capacity) {
			capacity *= 2;
			T *new_data = (T *)arena.allocate<T>(capacity * sizeof(T));
			std::memcpy(new_data, data, size * sizeof(T));
			data = new_data;

		}
		data[size++] = val;
	}

	void clear() {
		capacity = 16;
		size = 0;
		data = (T *)arena.allocate<T>(capacity * sizeof(T));
	}

	T &operator[](size_t index) {
		if (index >= capacity) {
			throw std::out_of_range("Index out of bounds");
		}
		return data[index];
	}

	const T &operator[](size_t index) const {
		if (index >= capacity) {
			throw std::out_of_range("Index out of bounds");
		}
		return data[index];
	}

};


template<typename T>
T *arena_alloc(Arena &arena)
{
	return new (arena.allocate<T>(sizeof(T))) T();
}

template<typename T>
Array<T> *new_array(Arena &arena, size_t init_cap = 16)
{
	return new (arena.allocate<Array<T>>(sizeof(Array<T>))) Array<T>(arena, init_cap);
}

inline
bool has_extension(const std::string &path, const std::string &ext)
{
	size_t dot_pos = path.find_last_of('.');
	if (dot_pos == std::string::npos || dot_pos == 0) return false;
	return path.substr(dot_pos) == ext;
}

inline
std::string remove_extension(const std::string &path)
{
	size_t dot_pos = path.find_last_of('.');
	if (dot_pos == std::string::npos || dot_pos == 0) return path;
	return path.substr(0, dot_pos);
}

char *string_alloc(Arena &arena, char value[256]);
char *string_alloc(Arena &arena, char value);

std::vector<std::string> parse_lines(std::string source);
