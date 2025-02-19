#pragma once
#include <iostream>
#include "Types.h"

namespace flo {
	///<summary>
	/// A class that allocates a pointer, which will be safely diposed of at the end of the objects life.
	/// However, the pointer can be freed, making it independant from the TempPtr object.
	///</summary>
	template<typename T>
	struct TempPtr {
	private:
		T* ptr = nullptr;
		bool locked = true;
		inline static T* empty = new T();
	public:
		TempPtr() {
			ptr = new T();
		}

		TempPtr(T* value) {
			ptr = value;
		}

		T& operator*() {
			if (!ptr) return empty;
			return *ptr;
		}

		T* operator->() {
			if (!ptr) return empty;
			return ptr;
		}

		void operator=(const T& value) {
			if (ptr) *ptr = value;
		}

		void operator=(T* value) {
			if (ptr && locked) delete ptr;
			ptr = value;
			locked = false;
		}

		operator bool() {
			return ptr;
		}

		void free() {
			locked = false;
		}

		void lock(T* value) {
			if (ptr && locked) delete ptr;
			locked = true;
			ptr = value;
		}

		T* getPtr() {
			if (!ptr) return empty;
			return ptr;
		}

		~TempPtr() {
			if (ptr && locked) {
				delete ptr;
			}
			ptr = nullptr;
		}
	};

	///<summary>
	/// A class containing, allocating and deleting a pointer.
	///</summary>
	template<typename T>
	struct UniquePtr {
	private:
		inline static T empty;
		T* ptr = nullptr;

	public:
		UniquePtr() = default;

		UniquePtr(const UniquePtr<T>& copy) {
			if (copy.ptr) ptr = new T(copy.ptr);
		}

		void operator<<(T* right) {
			if (ptr) delete ptr;
			ptr = right;
		}

		void operator=(const UniquePtr<T>& right) {
			if (this == &right) return;
			if (right.ptr) {
				if (ptr) *ptr = *right.ptr;
				else ptr = new T(right.ptr);
			}
			else {
				if (ptr) delete ptr;
				ptr = nullptr;
			}
		}

		void operator=(const T* right) {
			if (right) {
				if (ptr) *ptr = *right;
				else ptr = new T(*right);
			}
			else {
				if (ptr) delete ptr;
				ptr = nullptr;
			}
		}

		void swap(T*& right) {
			T* previous = ptr;
			ptr = right;
			right = previous;
		}

		void swap(UniquePtr<T>& right) {
			T* previous = ptr;
			ptr = right.ptr;
			right.ptr = previous;
		}

		constexpr T& operator&() const {
			if (ptr) return *ptr;
#if (_DEBUG)
			std::cerr << "Warning: Nullpointer dereference\n";
#endif
			return empty;
		}

		constexpr T* operator->() const {
			if (ptr) return ptr;
#if (_DEBUG)
			std::cerr << "Warning: Nullpointer dereference\n";
#endif
			return &empty;
		}

		T* getPtr() {
			if (!ptr) return &empty;
			return ptr;
		}

		constexpr operator bool() const {
			return ptr;
		}

		void dispose() {
			if (ptr) delete ptr;
			ptr = nullptr;
		}

		~UniquePtr() {
			dispose();
		}
	};

	///<summary>
	/// A class that contains simple array pointers.
	///</summary>
	template<typename T>
	struct Array {
	private:
		inline static T empty;
		T* ptr = nullptr;
		uint length = 0, free = 0;

	public:
		Array() = default;

		Array(uint size) : length(size) {
			ptr = new T[size];
		}

		Array(const Array& copy) {
			if (copy.length) {
				ptr = new T[copy.length];
				length = copy.length;
				free = copy.length;
				std::copy(copy.ptr, copy.ptr + length, ptr);
			}
		}

		void operator=(const Array<T>& right) {
			if (length != right.length) resize(right.length);
			if (!length) return;
			if (ptr && right.ptr) std::copy(right.ptr, right.ptr + length, ptr);
		}

		void swap(T*& right, uint& _length, uint& _free) {
			T* pre_ptr = ptr;
			uint pre_length = length;
			uint pre_free = free;
			ptr = right;
			length = _length;
			free = _free;
			right = pre_ptr;
			_length = pre_length;
			_free = pre_free;
		}

		void swap(Array<T>& right) {
			T* pre_ptr = ptr;
			uint pre_length = length;
			uint pre_free = free;
			ptr = right.ptr;
			length = right.length;
			free = right.free;
			right.ptr = pre_ptr;
			right.length = pre_length;
			right.free = pre_free;
		}

		T& operator[](int index) {
			if (index < 0 || index >= length) {
#if (_DEBUG)
				std::cerr << "Warning: Array index out of bounds\n";
#endif
				return empty;
			}
			return ptr[index];
		}

		constexpr uint size() const {
			return length;
		}

		void resize(uint new_size) {
			if (new_size > free) {
				if (ptr) delete[] ptr;
				ptr = new T[new_size];
				length = new_size;
				free = new_size;
			}
			else {
				length = new_size;
			}
		}

		void truncate() {
			if (length == free) return;
			else {
				T* old_ptr = ptr;
				ptr = new T[length];
				free = length;
				if (old_ptr) {
					std::copy(old_ptr, old_ptr + length, ptr);
					delete[] old_ptr;
				}
			}
		}

		T* getPtr() const {
			if (!ptr) return &empty;
			return ptr;
		}

		constexpr operator bool() const {
			return length;
		}

		~Array() {
			if (ptr) delete[] ptr;
			ptr = nullptr;
			length = 0;
			free = 0;
		}
	};
}