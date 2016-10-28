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
#include <ecstasy/utils/DefaultMemoryManager.hpp>

using ecstasy::MemoryPage;
using ecstasy::MemoryPageManager;
using ecstasy::DefaultMemoryManager;

#define NS_TEST_CASE(name) TEST_CASE("MemoryManager: " name)
namespace MemoryManagerTests {
	NS_TEST_CASE("page_allocate_free") {
		{
			MemoryPage page(sizeof(uint64_t));

			std::vector<void*> memories;
			for(int i=0; i<64; i++) {
				auto memory = page.allocate();
				REQUIRE(page.owns(memory));
				memories.push_back(memory);
			}
			REQUIRE(page.bitflags == 0);
			REQUIRE(page.freeUnits == 0);

			for(auto memory: memories) {
				REQUIRE(page.owns(memory));
				page.free(memory);
			}

			REQUIRE(page.bitflags == 0xFFFFFFFFFFFFFFFF);
			REQUIRE(page.freeUnits == 64);
		}
		REQUIRE(!MemoryPage::memoryLeakDetected);
		MemoryPage::memoryLeakDetected = false;
	}

	NS_TEST_CASE("page_allocate_too_much") {
		{
			MemoryPage page(sizeof(uint64_t));

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
		}
		REQUIRE(!MemoryPage::memoryLeakDetected);
		MemoryPage::memoryLeakDetected = false;
	}

	NS_TEST_CASE("page_free_twice") {
		{
			MemoryPage page(sizeof(uint64_t));

			auto memory = page.allocate();
			page.free(memory);
			bool exceptionCaught = false;
			try {
				page.free(memory);
			} catch(std::invalid_argument e) {
				exceptionCaught = true;
			}
			REQUIRE(exceptionCaught);
		}
		REQUIRE(!MemoryPage::memoryLeakDetected);
		MemoryPage::memoryLeakDetected = false;
	}

	NS_TEST_CASE("page_memory_leak") {
		REQUIRE(!MemoryPage::memoryLeakDetected);
		{
			MemoryPage page(sizeof(uint64_t));
			page.allocate();
		}
		REQUIRE(MemoryPage::memoryLeakDetected);
		MemoryPage::memoryLeakDetected = false;
	}

	NS_TEST_CASE("page_manager_allocate_free") {
		{
			MemoryPageManager manager(sizeof(uint64_t));
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
		}
		REQUIRE(!MemoryPage::memoryLeakDetected);
		MemoryPage::memoryLeakDetected = false;
	}

	NS_TEST_CASE("page_manager_free_twice") {
		{
			MemoryPageManager manager(sizeof(uint64_t));

			auto memory = manager.allocate();
			manager.free(memory);
			bool exceptionCaught = false;
			try {
				manager.free(memory);
			} catch(std::invalid_argument e) {
				exceptionCaught = true;
			}
			REQUIRE(exceptionCaught);
		}
		REQUIRE(!MemoryPage::memoryLeakDetected);
		MemoryPage::memoryLeakDetected = false;
	}

	NS_TEST_CASE("page_manager_free_invalid") {
		{
			MemoryPageManager manager(sizeof(uint64_t));

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
		}
		REQUIRE(!MemoryPage::memoryLeakDetected);
		MemoryPage::memoryLeakDetected = false;
	}

	NS_TEST_CASE("memory_manager_allocate_free") {
		{
			DefaultMemoryManager manager;
			REQUIRE(manager.getAllocationCount() == 0);
			REQUIRE(manager.getPageManagerCount() == 0);

			auto mem64 = manager.allocate(sizeof(uint64_t));
			REQUIRE(manager.getPageManagerCount() == 1);

			auto mem32 = manager.allocate(sizeof(uint32_t));
			REQUIRE(manager.getPageManagerCount() == 2);
			REQUIRE(manager.getAllocationCount(sizeof(uint64_t)) == 1);
			REQUIRE(manager.getAllocationCount(sizeof(uint32_t)) == 1);

			bool exceptionCaught = false;
			try {
				manager.free(sizeof(uint16_t), mem64);
			} catch(std::invalid_argument e) {
				exceptionCaught = true;
			}
			REQUIRE(exceptionCaught);

			manager.free(sizeof(uint64_t), mem64);
			REQUIRE(manager.getAllocationCount(sizeof(uint64_t)) == 0);
			manager.free(sizeof(uint32_t), mem32);
			REQUIRE(manager.getAllocationCount(sizeof(uint32_t)) == 0);

			manager.reduceMemory();
			REQUIRE(manager.getAllocationCount() == 0);
			REQUIRE(manager.getPageManagerCount() == 0);
		}
		REQUIRE(!MemoryPage::memoryLeakDetected);
		MemoryPage::memoryLeakDetected = false;
	}
}
