#pragma once

#include <stdint.h>
#include <memory>
#include <vector>
#include <set>

namespace ecstasy {
	//Fixme: in debug, mark bytes on creation, allocation, free?
	struct MemoryPage {
		uint32_t unitSize;
		uint8_t freeUnits = 64;
		char* memory;
		char* dataStart;
		char* dataEnd;
		uint64_t bitflags = 0xFFFFFFFFFFFFFFFF;
		uint64_t dataOffset = 0; // to be used for alignment

		MemoryPage(uint32_t unitSize);
		~MemoryPage();

		/// @return memory with enough size, or nullptr if no free memory is available
		void* allocate();
		/// @return true if memory belongs to this page
		bool owns(void* memory);
		/// @return false if memory already free
		bool free(void* memory);
	};

	class MemoryManager {
	private:
		uint32_t unitSize;
		std::vector<std::unique_ptr<MemoryPage>> pages;
		std::vector<MemoryPage*> freePages;

	public:
		MemoryManager(uint32_t unitSize) : unitSize(unitSize) {}
		MemoryManager(const MemoryManager &) = delete;
		~MemoryManager() {}

		void* allocate();
		bool free(void* memory);
	};
}