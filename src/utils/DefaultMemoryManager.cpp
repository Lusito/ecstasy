#include <ecstasy/utils/DefaultMemoryManager.hpp>

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
			throw std::invalid_argument("Trying to allocate memory on a full MemoryPage");
		int index = getFirstSetBit(bitflags);

		if(((bitflags >> index) & 1ull) == 0)
			throw std::invalid_argument("Trying to allocate memory which is already allocated");

		void* data = memory + dataOffset + index*unitSize;
		bitflags ^= 1ull << index;
		freeUnits--;

		return data;
	}

	bool MemoryPage::owns(void* memory) {
		return memory >= dataStart && memory < dataEnd;
	}

	void MemoryPage::free(void* memory) {
		int index = static_cast<uint32_t>(static_cast<char *>(memory) - dataStart) / unitSize;

		if(((bitflags >> index) & 1ull) != 0)
			throw std::invalid_argument("Trying to free memory which has already been freed");

		bitflags ^= 1ull << index;
		freeUnits++;
	}

	void* MemoryPageManager::allocate() {
		if(freePages.empty()) {
			pages.emplace_back(std::make_unique<MemoryPage>(unitSize));
			freePages.push_back(pages.back().get());
		}
		auto page = freePages.back();
		void *result = page->allocate();
		allocationCount++;
		if(!page->freeUnits)
			freePages.pop_back();
		return result;
	}

	void MemoryPageManager::free(void* memory) {
		for(auto& page: pages) {
			if(page->owns(memory)) {
				page->free(memory);
				allocationCount--;
				if(page->freeUnits == 1)
					freePages.push_back(page.get());
				return;
			}
		}
		throw std::invalid_argument("Trying to free memory which does not belong to this memory manager");
	}

	void MemoryPageManager::reduceMemory() {
		for (auto it = pages.cbegin(); it != pages.cend();) {
			if((*it)->freeUnits == 64) {
				it = pages.erase(it);
			} else {
				++it;
			}
		}
		freePages.clear();
	}

	void* DefaultMemoryManager::allocate(uint32_t size) {
		auto it = managers.find(size);
		MemoryPageManager *manager;
		if(it != managers.end())
			manager = it->second.get();
		else
			manager = managers.emplace(size, std::make_unique<MemoryPageManager>(size)).first->second.get();
		return manager->allocate();
	}

	void DefaultMemoryManager::free(uint32_t size, void* memory) {
		auto it = managers.find(size);
		if(it == managers.end())
			throw std::invalid_argument("Trying to free memory which does not belong to this memory manager");
		else {
			auto manager = it->second.get();
			manager->free(memory);
		}
	}

	void DefaultMemoryManager::reduceMemory() {
		for (auto it = managers.cbegin(); it != managers.cend();) {
			if(it->second->getAllocationCount() != 0)
				it->second->reduceMemory();
			if(it->second->getAllocationCount() == 0) {
				it = managers.erase(it);
			} else {
				++it;
			}
		}
	}

	uint32_t DefaultMemoryManager::getAllocationCount() const {
		uint32_t count = 0;
		for(auto& kv: managers)
			count += kv.second->getAllocationCount();
		return count;
	}
}
