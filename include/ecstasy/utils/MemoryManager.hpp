#pragma once

#include <stdint.h>

namespace ecstasy {
	class MemoryManager {
	public:
		virtual void* allocate(uint32_t size) = 0;
		virtual void free(uint32_t size, void* memory) = 0;
		virtual void reduceMemory() = 0;
		virtual uint32_t getAllocationCount() const = 0;
	};
}
