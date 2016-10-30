/*******************************************************************************
 * Copyright 2015 See AUTHORS file.
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
#include "../TestBase.hpp"
#include <ecstasy/utils/alignof.hpp>
#include <emmintrin.h>

using ecstasy::MemoryPageManager;
using ecstasy::DefaultMemoryManager;

#define NS_TEST_CASE(name) TEST_CASE("MemoryManager: " name)
namespace MemoryManagerTests {
	NS_TEST_CASE("page_allocate_free") {
		TEST_MEMORY_LEAK_START
		MemoryPage page(0, sizeof(uint64_t), alignof(uint64_t));

		std::vector<void*> memories;
		for(int i=0; i<64; i++) {
			auto memory = page.allocate();
			REQUIRE(page.owns(memory));
			memories.push_back(memory);
		}
		REQUIRE(page.getBitflags() == 0);
		REQUIRE(page.getFreeUnits() == 0);

		for(auto memory: memories) {
			REQUIRE(page.owns(memory));
			page.free(memory);
		}

		REQUIRE(page.getBitflags() == 0xFFFFFFFFFFFFFFFF);
		REQUIRE(page.getFreeUnits() == 64);
		TEST_MEMORY_LEAK_END
	}

	NS_TEST_CASE("page_allocate_too_much") {
		TEST_MEMORY_LEAK_START
		MemoryPage page(0, sizeof(uint64_t), alignof(uint64_t));

		std::vector<void*> memories;
		for(int i=0; i<64; i++) {
			auto memory = page.allocate();
			REQUIRE(page.owns(memory));
			memories.push_back(memory);
		}

		bool exceptionCaught = false;
		try {
			page.allocate();
		} catch(std::bad_alloc e) {
			exceptionCaught = true;
		}
		REQUIRE(exceptionCaught);

		for(auto memory: memories) {
			REQUIRE(page.owns(memory));
			page.free(memory);
		}
		TEST_MEMORY_LEAK_END
	}

	NS_TEST_CASE("page_free_twice") {
		TEST_MEMORY_LEAK_START
		MemoryPage page(0, sizeof(uint64_t), alignof(uint64_t));

		auto memory = page.allocate();
		page.free(memory);
		bool exceptionCaught = false;
		try {
			page.free(memory);
		} catch(std::invalid_argument e) {
			exceptionCaught = true;
		}
		REQUIRE(exceptionCaught);
		TEST_MEMORY_LEAK_END
	}

	NS_TEST_CASE("page_memory_leak") {
		REQUIRE(!MemoryPage::memoryLeakDetected);
		{
			MemoryPage page(0, sizeof(uint64_t), alignof(uint64_t));
			page.allocate();
		}
		REQUIRE(MemoryPage::memoryLeakDetected);
		MemoryPage::memoryLeakDetected = false;
	}

	NS_TEST_CASE("page_manager_allocate_free") {
		TEST_MEMORY_LEAK_START
		MemoryPageManager manager(sizeof(uint64_t), alignof(uint64_t));
		REQUIRE(manager.getAllocationCount() == 0);
		REQUIRE(manager.getPageCount() == 0);

		auto first = manager.allocate();
		std::vector<void*> memories;
		for(int i=0; i<63; i++)
			memories.push_back(manager.allocate());

		REQUIRE(manager.getAllocationCount() == 64);
		REQUIRE(manager.getPageCount() == 1);

		auto last = manager.allocate();
		REQUIRE(manager.getAllocationCount() == 65);
		REQUIRE(manager.getPageCount() == 2);

		manager.free(first);
		manager.reduceMemory();
		REQUIRE(manager.getAllocationCount() == 64);
		REQUIRE(manager.getPageCount() == 2);

		manager.free(last);
		REQUIRE(manager.getAllocationCount() == 63);
		REQUIRE(manager.getPageCount() == 2);

		manager.reduceMemory();
		REQUIRE(manager.getAllocationCount() == 63);
		REQUIRE(manager.getPageCount() == 1);

		memories.push_back(manager.allocate());
		REQUIRE(manager.getAllocationCount() == 64);
		REQUIRE(manager.getPageCount() == 1);

		for(auto memory: memories)
			manager.free(memory);
		TEST_MEMORY_LEAK_END
	}

	NS_TEST_CASE("page_manager_metadata_update") {
		TEST_MEMORY_LEAK_START
		MemoryPageManager manager(sizeof(uint64_t), alignof(uint64_t));

		// Allocate enough for 2 pages
		std::vector<void*> memories;
		for(int i=0; i<64; i++)
			memories.push_back(manager.allocate());

		auto last = manager.allocate();
		REQUIRE(manager.getAllocationCount() == 65);
		REQUIRE(manager.getPageCount() == 2);

		// free all of the first page and reduce memory
		for(auto memory: memories)
			manager.free(memory);
		manager.reduceMemory();

		REQUIRE(manager.getAllocationCount() == 1);
		REQUIRE(manager.getPageCount() == 1);

		// see if metadata is still in tact to free the last item which was in page 2
		manager.free(last);
		TEST_MEMORY_LEAK_END
	}

	NS_TEST_CASE("page_manager_free_twice") {
		TEST_MEMORY_LEAK_START
		MemoryPageManager manager(sizeof(uint64_t), alignof(uint64_t));

		auto memory = manager.allocate();
		manager.free(memory);
		bool exceptionCaught = false;
		try {
			manager.free(memory);
		} catch(std::invalid_argument e) {
			exceptionCaught = true;
		}
		REQUIRE(exceptionCaught);
		TEST_MEMORY_LEAK_END
	}

	NS_TEST_CASE("page_manager_free_invalid") {
		TEST_MEMORY_LEAK_START
		MemoryPageManager manager(sizeof(uint64_t), alignof(uint64_t));

		auto memory = manager.allocate();
		bool exceptionCaught = false;
		try {
			uint64_t test;
			manager.free(&test);
		} catch(std::invalid_argument e) {
			exceptionCaught = true;
		}
		REQUIRE(exceptionCaught);
		manager.free(memory);
		TEST_MEMORY_LEAK_END
	}

	NS_TEST_CASE("default_memory_manager_allocate_free") {
		TEST_MEMORY_LEAK_START
		DefaultMemoryManager manager;
		REQUIRE(manager.getAllocationCount() == 0);
		REQUIRE(manager.getPageManagerCount() == 0);

		auto mem64 = manager.allocate(sizeof(uint64_t), alignof(uint64_t));
		REQUIRE(manager.getPageManagerCount() == 1);

		auto mem32 = manager.allocate(sizeof(__m128), alignof(__m128));
		REQUIRE(manager.getPageManagerCount() == 2);
		REQUIRE(manager.getAllocationCount(sizeof(uint64_t), alignof(uint64_t)) == 1);
		REQUIRE(manager.getAllocationCount(sizeof(__m128), alignof(__m128)) == 1);

		bool exceptionCaught = false;
		try {
			manager.free(64, 64, mem64);
		} catch(std::invalid_argument e) {
			exceptionCaught = true;
		}
		REQUIRE(exceptionCaught);

		manager.free(sizeof(uint64_t), alignof(uint64_t), mem64);
		REQUIRE(manager.getAllocationCount(sizeof(uint64_t), alignof(uint64_t)) == 0);
		manager.free(sizeof(__m128), alignof(__m128), mem32);
		REQUIRE(manager.getAllocationCount(sizeof(__m128), alignof(__m128)) == 0);

		manager.reduceMemory();
		REQUIRE(manager.getAllocationCount() == 0);
		REQUIRE(manager.getPageManagerCount() == 0);
		TEST_MEMORY_LEAK_END
	}

	NS_TEST_CASE("default_page_manager_alignment") {
		TEST_MEMORY_LEAK_START
		uint32_t size = 20;
		uint32_t align = 8;

		DefaultMemoryManager manager;

		std::vector<void*> memories;
		for(int i=0; i<128; i++) {
			auto mem = manager.allocate(size, align);
			REQUIRE((reinterpret_cast<uint64_t>(mem) % align) == 0);
			memories.push_back(mem);
		}

		for(auto memory: memories)
			manager.free(size, align, memory);
		TEST_MEMORY_LEAK_END
	}
}
