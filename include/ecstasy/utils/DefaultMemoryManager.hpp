#pragma once

#include <memory>
#include <vector>
#include <map>
#include <ecstasy/utils/MemoryManager.hpp>

namespace ecstasy {
	//Fixme: in debug, mark bytes on creation, allocation, free?
	struct MemoryPage {
		static bool memoryLeakDetected;
		uint32_t unitSize;
		uint8_t freeUnits = 64;
		char* memory;
		char* dataStart;
		char* dataEnd;
		uint64_t bitflags = 0xFFFFFFFFFFFFFFFF;
		uint64_t dataOffset = 0; // to be used for alignment

		MemoryPage(uint32_t unitSize);
		~MemoryPage();

		/// @return New memory with enough size
		void* allocate();
		/// @return @a true if memory belongs to this page
		bool owns(void* memory);
		/// @return @a false if memory already free
		void free(void* memory);
	};

	class MemoryPageManager {
	private:
		uint32_t unitSize;
		uint32_t allocationCount = 0;
		std::vector<std::unique_ptr<MemoryPage>> pages;
		std::vector<MemoryPage*> freePages;

	public:
		MemoryPageManager(uint32_t unitSize) : unitSize(unitSize) {}
		MemoryPageManager(const MemoryPageManager &) = delete;
		~MemoryPageManager() {}

		uint32_t getAllocationCount() const {
			return allocationCount;
		}

		uint32_t getPageCount() const {
			return pages.size();
		}

		void* allocate();
		void free(void* memory);

		void reduceMemory();
	};

	class DefaultMemoryManager : public MemoryManager {
	private:
		std::map<uint32_t, std::unique_ptr<MemoryPageManager>> managers;

	public:
		DefaultMemoryManager() {}
		DefaultMemoryManager(const DefaultMemoryManager &) = delete;
		~DefaultMemoryManager() {}

		void* allocate(uint32_t size) override;
		void free(uint32_t size, void* memory) override;

		void reduceMemory() override;

		uint32_t getAllocationCount() const override;

		uint32_t getPageManagerCount() const;
		uint32_t getAllocationCount(uint32_t size) const;
		uint32_t getPageCount(uint32_t size) const;
	};
}
