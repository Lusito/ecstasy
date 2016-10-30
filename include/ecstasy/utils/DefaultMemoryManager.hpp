#pragma once
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

#include <memory>
#include <vector>
#include <map>
#include <ecstasy/utils/MemoryManager.hpp>

namespace ecstasy {
	/**
	 * Unit size must be the memory size plus some metadata-bytes, adjusted to a multiple of the memory alignment
	 *
	 * @param size The memory size to adjust
	 * @param align The memory alignment to adjust to
	 * @return The adjusted value
	 */
	uint32_t getMemoryUnitSize(uint32_t size, uint32_t align);

	/**
	 * A structure to hold memory for up to 64 allocations of a specified unit-size
	 *
	 * @todo in debug build, mark bytes as clear (0xCC), allocated(0xAA), deallocated(0xDD)?
	 */
	class MemoryPage {
	public:
		/// Set to true when {@~MemoryPage()} detects a memory leak.
		static bool memoryLeakDetected;

	private:
		uint16_t listIndex;
		uint32_t unitSize;
		uint8_t freeUnits = 64;
		char* memory;
		char* dataStart;
		char* dataEnd;
		uint64_t bitflags = 0xFFFFFFFFFFFFFFFF;
		uint64_t dataOffset = 0;

	public:
		/**
		 * @param listIndex The index of this page in MemoryPageManager
		 * @param unitSize The unit size to allocate. Use getMemoryUnitSize()
		 * @param align The memory alignment to adjust to
		 */
		MemoryPage(uint16_t listIndex, uint32_t unitSize, uint32_t align);
		~MemoryPage();

		/**
		 * Allocate enough memory for the unit-size.
		 *
		 * @return A pointer to the allocated memory.
		 * @throws std::bad_alloc when the allocation could not be made.
		 */
		void* allocate();

		/**
		 * Check if the specified memory belongs to this page.
		 *
		 * @return @a true if memory belongs to this page
		 * @param memory The memory to check.
		 */
		bool owns(void* memory);

		/**
		 * Free previously allocated memory.
		 *
		 * @param memory The memory to be freed.
		 */
		void free(void* memory);

		/// @return The index of this page
		uint16_t getListIndex() const {
			return listIndex;
		}

		/// @param newIndex The index of this page
		void setListIndex(uint16_t newIndex);

		/// @return The number of free memory units.
		uint8_t getFreeUnits() const {
			return freeUnits;
		}

		/// @return Bitmap to show which memory unit are free.
		uint64_t getBitflags() const {
			return bitflags;
		}
	};

	/**
	 * Manages {@link MemoryPage}s for one unit-size.
	 */
	class MemoryPageManager {
	private:
		uint32_t unitSize;
		uint32_t align;
		uint32_t allocationCount = 0;
		std::vector<std::unique_ptr<MemoryPage>> pages;
		std::vector<MemoryPage*> freePages;

	public:
		/**
		 * Default constructor
		 *
		 * @param unitSize The unit size to allocate. Use getMemoryUnitSize()
		 * @param align The memory alignment to adjust to
		 */
		MemoryPageManager(uint32_t unitSize, uint32_t align) : unitSize(unitSize), align(align) {}
		MemoryPageManager(const MemoryPageManager &) = delete;
		~MemoryPageManager() {}

		/// @return The number of allocations currently in use.
		uint32_t getAllocationCount() const {
			return allocationCount;
		}

		/// @return The number of {@link MemoryPage}s currently in use.
		uint32_t getPageCount() const {
			return pages.size();
		}

		/**
		 * Allocate enough memory for the unit-size.
		 *
		 * @return A pointer to the allocated memory.
		 * @throws std::bad_alloc when the allocation could not be made.
		 */
		void* allocate();

		/**
		 * Free previously allocated memory.
		 *
		 * @param memory The memory to be freed.
		 */
		void free(void* memory);

		/// Try to reduce the memory footprint if possible.
		void reduceMemory();
	};

	/**
	 * The default memory manager.
	 * It creates one MemoryPageManager for each size.
	 */
	class DefaultMemoryManager : public MemoryManager {
	private:
		std::map<uint32_t, std::unique_ptr<MemoryPageManager>> managers;

	public:
		DefaultMemoryManager() {}
		DefaultMemoryManager(const DefaultMemoryManager &) = delete;
		~DefaultMemoryManager() {}

		void* allocate(uint32_t size, uint32_t align) override;
		void free(uint32_t size, uint32_t align, void* memory) override;
		void reduceMemory() override;
		uint32_t getAllocationCount() const override;

		/// @return The number of {@link MemoryPageManager}s currently in use.
		uint32_t getPageManagerCount() const;

		/**
		 * Call MemoryPageManager::getAllocationCount() on the MemoryPageManager used for the specified size.
		 *
		 * @param size The size used to allocate memory.
		 * @param align The align used to allocate memory.
		 * @return The number of allocations currently in use for the specified size.
		 */
		uint32_t getAllocationCount(uint32_t size, uint32_t align) const;

		/**
		 * Call MemoryPageManager::getPageCount() on the MemoryPageManager used for the specified size.
		 *
		 * @param size The size used to allocate memory.
		 * @param align The align used to allocate memory.
		 * @return The number of {@link MemoryPage}s currently in use for the specified size.
		 */
		uint32_t getPageCount(uint32_t size, uint32_t align) const;
	};
}
