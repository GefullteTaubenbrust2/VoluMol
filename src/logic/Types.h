#pragma once
#include <typeindex>
#include <vector>
#include <cstddef>

typedef std::int64_t i64;
typedef std::int32_t i32;
typedef std::int16_t i16;
typedef std::int8_t i8;
typedef std::uint8_t u8;
typedef std::uint16_t u16;
typedef std::uint32_t u32;
typedef std::uint64_t u64;
typedef std::size_t size;
typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;

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