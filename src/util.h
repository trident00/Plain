#pragma once
#include <vector>
#ifdef _DEBUG
#include <iostream>
#endif
#include <stdexcept>
#include <string>

#ifdef _WIN32
#include "Windows.h"
inline void set_color(int color) {
	HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(h, color);
}
#define RESET   (set_color(7), "")
#define RED     (set_color(4), "")
#define BLUE    (set_color(1), "")
#define CYAN    (set_color(3), "")
#else
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define BLUE    "\033[34m"
#define CYAN    "\033[36m" 
#endif

#define Foreach(arr) for (int i = 0; i < arr.size; ++i)

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
	Array(Arena &a, bool alloc = true, size_t cap=16) : using_stack(false), capacity(16) {
		arena = &a;
		if (alloc) allocate(cap);
	}

	void allocate(size_t cap) {
		data = (T *)arena->allocate<T>(cap * sizeof(T));
		capacity = cap;
	}

	bool using_stack;
	Arena *arena;
	size_t capacity;
	size_t size = 0;

	T *data = nullptr;

	void push(const T &val) {
		if (size >= capacity) {
			capacity *= 2;
			T *new_data = (T *)arena->allocate<T>(capacity * sizeof(T));
			std::memcpy(new_data, data, size * sizeof(T));
			data = new_data;
		}
		data[size++] = val;
	}

	T &peek() { return data[size]; }

	void clear() {
		size = 0;
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

inline
void test_array()
{
	Arena arena = Arena(1024 * 1024);
	Array<int> arr = Array<int>(arena);
	arr.push(2);

}


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
