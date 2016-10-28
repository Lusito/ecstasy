#include <ecstasy/utils/MemoryManager.hpp>

namespace ecstasy {
	int getFirstSetBit(uint64_t bits) {
		static const char multiplyDeBruijnBitPosition[64] = {
			0, 1, 2, 56, 3, 32, 57, 46, 29, 4, 20, 33, 7, 58, 11, 47,
			62, 30, 18, 5, 16, 21, 34, 23, 53, 8, 59, 36, 25, 12, 48, 39,
			63, 55, 31, 45, 28, 19, 6, 10, 61, 17, 15, 22, 52, 35, 24, 38,
			54, 44, 27, 9, 60, 14, 51, 37, 43, 26, 13, 50, 42, 49, 41, 40
		};
		return multiplyDeBruijnBitPosition[((uint64_t) ((bits & -bits) * 0x26752B916FC7B0DULL)) >> 58];
	}

	MemoryPage::MemoryPage(uint32_t unitSize)
		: unitSize(unitSize) {
		uint32_t memorySize = unitSize*64;
		memory = new char[memorySize]; //fixme: some more for alignment
		//Fixme: calculate dataOffset
		dataStart = memory + dataOffset;
		dataEnd = dataStart + memorySize;
	}

	MemoryPage::~MemoryPage() {
		delete[] memory;
	}

	void* MemoryPage::allocate() {
		if (!bitflags)
			return nullptr;
		int index = getFirstSetBit(bitflags);
		
		if(((bitflags >> index) & 1) == 0)
			return nullptr; // should never happen
		
		void* data = memory + dataOffset + index*unitSize;
		bitflags ^= 1 << index;
		
		return data;
	}

	bool MemoryPage::owns(void* memory) {
		return memory >= dataStart && memory < dataEnd;
	}

	bool MemoryPage::free(void* memory) {
		int index = static_cast<uint32_t>(static_cast<char *>(memory) - dataStart) / unitSize;

		if(((bitflags >> index) & 1) == 0)
			return false;

		bitflags ^= 1 << index;
		return true;
	}

	void* MemoryManager::allocate() {
		if(freePages.empty()) {
			pages.emplace_back(std::make_unique<MemoryPage>(unitSize));
			freePages.push_back(pages.back().get());
		}
		auto page = freePages.back();
		void *result = page->allocate();
		if(!page->freeUnits)
			freePages.pop_back();
		return result;
	}

	bool MemoryManager::free(void* memory) {
		for(auto& page: pages) {
			if(page->owns(memory)) {
				if(page->free(memory)) {
					if(page->freeUnits == 1)
						freePages.push_back(page.get());
					return true;
				}
				return false;
			}
		}
		// not found
		return false;
	}
}