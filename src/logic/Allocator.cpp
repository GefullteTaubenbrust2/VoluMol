#include "Allocator.h"
#include <iostream>

namespace flo {
	void* ContiguousAllocator::allocate(size s) {
		uint next_size = block_size;
		if (s > next_size) next_size = s;
		if (!allocations.size()) {
			AllocatorBlock block;
			block.data = new u8[next_size];
			block.free = next_size - s;
			allocations.push_back(block);
			memory = block.data + s;
			return block.data;
		}
		AllocatorBlock& current = allocations[allocations.size() - 1];
		if (current.free < s) {
			AllocatorBlock block;
			block.data = new u8[next_size];
			block.free = next_size - s;
			allocations.push_back(block);
			memory = block.data + s;
			return block.data;
		}
		current.free -= s;
		void* data = memory;
		memory += s;
		return data;
	}

	void ContiguousAllocator::destroy_all() {
		for (int i = 0; i < allocations.size(); ++i) {
			delete[] allocations[i].data;
		}
	}
}
