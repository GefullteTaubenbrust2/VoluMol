#pragma once
#include <typeindex>
#include <vector>
#include <cstddef>

#if defined _WIN64 || defined __x86_64 || defined ppc64__
typedef long long i64;
typedef int i32;
typedef short i16;
typedef char i8;
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;
typedef unsigned long long size;
typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
#elif defined _WIN32
typedef long long i64;
typedef int i32;
typedef short i16;
typedef char i8;
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;
typedef unsigned int size;
typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
#endif

typedef size_t typehash;

///<summary>
/// Generate a unique hash for a type.
///</summary>
template<typename T>
typehash typeHash() {
	return std::type_index(typeid(T)).hash_code();
}

template<typename T>
std::vector<typehash> typeVector() {
	return std::vector<typehash>{typeHash<T>()};
}

template<typename T0, typename T1, typename ... Ts>
std::vector<typehash> typeVector() {
	std::vector<typehash> vector = typeVector<T1, Ts...>();
	vector.insert(vector.begin(), typeHash<T0>());
	return vector;
}

#define INVALID_INDEX 0xffffffff