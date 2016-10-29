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

#include <stdint.h>

namespace ecstasy {
	/**
	 * Memory manager interface. Used to allocate entities, components and helper structures for delayed operations.
	 */
	class MemoryManager {
	public:
		/**
		 * Allocate the specified amount of memory.
		 * 
		 * @param size The size of memory (in bytes) to allocate
		 * @return A pointer to the allocated memory.
		 * @throws std::bad_alloc when the allocation could not be made.
		 */
		virtual void* allocate(uint32_t size) = 0;

		/**
		 * Free previously allocated memory.
		 * 
		 * @param size The <b>same size</b>, which was used to allocate() the memory.
		 * @param memory The memory to be freed.
		 */
		virtual void free(uint32_t size, void* memory) = 0;

		/// Try to reduce the memory footprint if possible.
		virtual void reduceMemory() = 0;

		/// @return The number of allocations currently in use.
		virtual uint32_t getAllocationCount() const = 0;
	};
}
