#pragma once
#include "Types.h"
#include <vector>

namespace flo {
	#define CONTIGUOUS_NEW(allocator, pointer, type, constructor) { pointer = new(allocator.allocate(sizeof(type))) type##constructor;}

	struct AllocatorBlock {
		u8* data;
		uint free = 0;
	};

	struct ContiguousAllocator {
	protected:
		u8* memory = nullptr;
		std::vector<AllocatorBlock> allocations;

	public:
		size block_size = 2048;

		ContiguousAllocator() = default;

		void* allocate(size size);

		template<typename T>
		T* allocate() {
			void* data = allocate(sizeof(T));
			new(data) T();
			return (T*)data;
		}
	
		void destroy_all();
	};
}