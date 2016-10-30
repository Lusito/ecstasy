/*******************************************************************************
 * Copyright 2011 See AUTHORS file.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ******************************************************************************/
#include <ecstasy/utils/DefaultMemoryManager.hpp>
#include <algorithm>

namespace ecstasy {
	static const uint32_t MEMORY_META_SIZE = sizeof(uint16_t);

	uint32_t getMemoryUnitSize(uint32_t size, uint32_t align) {
		size += MEMORY_META_SIZE;
		if (size % align == 0)
			return size;
		return (size / align) * align + align;
	}

	int getFirstSetBit(uint64_t bits) {
		static const char multiplyDeBruijnBitPosition[64] = {
			0, 1, 2, 56, 3, 32, 57, 46, 29, 4, 20, 33, 7, 58, 11, 47,
			62, 30, 18, 5, 16, 21, 34, 23, 53, 8, 59, 36, 25, 12, 48, 39,
			63, 55, 31, 45, 28, 19, 6, 10, 61, 17, 15, 22, 52, 35, 24, 38,
			54, 44, 27, 9, 60, 14, 51, 37, 43, 26, 13, 50, 42, 49, 41, 40
		};
		return multiplyDeBruijnBitPosition[((uint64_t) ((bits & -bits) * 0x26752B916FC7B0DULL)) >> 58];
	}

	bool MemoryPage::memoryLeakDetected;
	MemoryPage::MemoryPage(uint16_t listIndex, uint32_t unitSize, uint32_t align)
		: listIndex(listIndex), unitSize(unitSize) {
		uint32_t memorySize = unitSize*64 + align;
		memory = new char[memorySize];
		auto rest = reinterpret_cast<uint64_t>(memory) % align;
		dataOffset = (align - rest);
		dataStart = memory + dataOffset;
		dataEnd = dataStart + memorySize;
	}

	MemoryPage::~MemoryPage() {
		delete[] memory;
		if(freeUnits != 64)
			memoryLeakDetected = true;
	}

	void* MemoryPage::allocate() {
		if (!bitflags)
			throw std::bad_alloc();
		int index = getFirstSetBit(bitflags);

		if(((bitflags >> index) & 1ull) == 0)
			throw std::bad_alloc();

		char* data = memory + dataOffset + index*unitSize;
		bitflags ^= 1ull << index;
		freeUnits--;

		uint16_t *metaData = reinterpret_cast<uint16_t *>(data + unitSize - MEMORY_META_SIZE);
		*metaData = listIndex;

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

	void MemoryPage::setListIndex(uint16_t newIndex) {
		if(listIndex != newIndex) {
			listIndex = newIndex;
			if(freeUnits != 64) {
				// Update metaData for all units
				char *data = dataStart;
				for(int i=0; i<64; i++) {
					data += unitSize;
					uint16_t *metaData = reinterpret_cast<uint16_t *>(data - MEMORY_META_SIZE);
					*metaData = listIndex;
				}
			}
		}
	}

	void* MemoryPageManager::allocate() {
		if(freePages.empty()) {
			pages.emplace_back(std::make_unique<MemoryPage>(static_cast<uint16_t>(pages.size()), unitSize, align));
			freePages.push_back(pages.back().get());
		}
		auto page = freePages.back();
		void *result = page->allocate();
		allocationCount++;
		if(!page->getFreeUnits())
			freePages.pop_back();
		return result;
	}

	void MemoryPageManager::free(void* memory) {
		uint16_t *metaData = reinterpret_cast<uint16_t *>(reinterpret_cast<char *>(memory) + unitSize - MEMORY_META_SIZE);
		if(*metaData < pages.size()) {
			auto owningPage = pages[*metaData].get();
			if(owningPage->owns(memory)) {
				owningPage->free(memory);
				allocationCount--;
				if(owningPage->getFreeUnits() == 1)
					freePages.push_back(owningPage);
				return;
			}
		}
		throw std::invalid_argument("Trying to free memory which does not belong to this memory manager");
	}

	void MemoryPageManager::reduceMemory() {
		uint16_t removed = 0;
		for (auto it = pages.cbegin(); it != pages.cend();) {
			auto page = it->get();
			if(page->getFreeUnits() == 64) {
				auto freeIt = std::find(freePages.begin(), freePages.end(), page);
				if (freeIt != freePages.end())
					freePages.erase(freeIt);
				it = pages.erase(it);
				removed++;
			} else {
				page->setListIndex(page->getListIndex() - removed);
				++it;
			}
		}
	}

	void* DefaultMemoryManager::allocate(uint32_t size, uint32_t align) {
		size = getMemoryUnitSize(size, align);
		uint64_t key = static_cast<uint64_t>(size) << 32 | align;
		auto it = managers.find(key);
		MemoryPageManager *manager;
		if(it != managers.end())
			manager = it->second.get();
		else
			manager = managers.emplace(key, std::make_unique<MemoryPageManager>(size, align)).first->second.get();
		return manager->allocate();
	}

	void DefaultMemoryManager::free(uint32_t size, uint32_t align, void* memory) {
		size = getMemoryUnitSize(size, align);
		uint64_t key = static_cast<uint64_t>(size) << 32 | align;
		auto it = managers.find(key);
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

	uint32_t DefaultMemoryManager::getPageManagerCount() const {
		return managers.size();
	}

	uint32_t DefaultMemoryManager::getAllocationCount(uint32_t size, uint32_t align) const {
		size = getMemoryUnitSize(size, align);
		uint64_t key = static_cast<uint64_t>(size) << 32 | align;
		auto it = managers.find(key);
		if(it == managers.end())
			return 0;
		return it->second.get()->getAllocationCount();
	}

	uint32_t DefaultMemoryManager::getPageCount(uint32_t size, uint32_t align) const {
		size = getMemoryUnitSize(size, align);
		uint64_t key = static_cast<uint64_t>(size) << 32 | align;
		auto it = managers.find(key);
		if(it == managers.end())
			return 0;
		return it->second.get()->getPageCount();
	}
}
