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
	 * A structure to hold memory for up to 64 allocations of a specified unit-size
	 * 
	 * @todo in debug build, mark bytes as clear (0xCC), allocated(0xAA), deallocated(0xDD)?
	 * @todo Align data correctly
	 */
	struct MemoryPage {
		/// Set to true when {@~MemoryPage()} detects a memory leak.
		static bool memoryLeakDetected;

		/// The unit size to allocate
		uint32_t unitSize;

		/// The number of free memory units.
		uint8_t freeUnits = 64;

		/// The original memory pointer to be deleted on destruction
		char* memory;

		/// memory + dataOffset
		char* dataStart;

		/// dataStart + unitSize*64
		char* dataEnd;

		/// Bitmap to show which memory unit are free.
		uint64_t bitflags = 0xFFFFFFFFFFFFFFFF;

		/// To correctly align memory, data must be offset. This stores how big the offset is.
		uint64_t dataOffset = 0;

		/// @param unitSize The unit size to allocate
		MemoryPage(uint32_t unitSize);
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
	};

	/**
	 * Manages {@link MemoryPage}s for one unit-size.
	 */
	class MemoryPageManager {
	private:
		uint32_t unitSize;
		uint32_t allocationCount = 0;
		std::vector<std::unique_ptr<MemoryPage>> pages;
		std::vector<MemoryPage*> freePages;

	public:
		/// @param unitSize The unit size to allocate
		MemoryPageManager(uint32_t unitSize) : unitSize(unitSize) {}
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

		void* allocate(uint32_t size) override;
		void free(uint32_t size, void* memory) override;
		void reduceMemory() override;
		uint32_t getAllocationCount() const override;

		/// @return The number of {@link MemoryPageManager}s currently in use.
		uint32_t getPageManagerCount() const;

		/**
		 * Call MemoryPageManager::getAllocationCount() on the MemoryPageManager used for the specified size.
		 * 
		 * @param size The size used to allocate memory.
		 * @return The number of allocations currently in use for the specified size.
		 */
		uint32_t getAllocationCount(uint32_t size) const;

		/**
		 * Call MemoryPageManager::getPageCount() on the MemoryPageManager used for the specified size.
		 * 
		 * @param size The size used to allocate memory.
		 * @return The number of {@link MemoryPage}s currently in use for the specified size.
		 */
		uint32_t getPageCount(uint32_t size) const;
	};
}
