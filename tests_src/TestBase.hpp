#pragma once
#include "catch.hpp"
#include <vector>
#include <memory>
#include <ecstasy/core/Engine.hpp>
#include <ecstasy/core/EntitySystem.hpp>
#include <ecstasy/core/Family.hpp>
#include <ecstasy/core/Component.hpp>
#include <ecstasy/utils/DefaultMemoryManager.hpp>

using ecstasy::Bits;
using ecstasy::getComponentType;

template<typename T>
class Allocator {
public:
	std::vector<std::shared_ptr<T>> values;

	T* create() {
		auto instance = std::shared_ptr<T>(new T());
		values.push_back(instance);
		return instance.get();
	}
};
template<typename T>
bool contains(const std::vector<T> &v, const T &value) {
	return std::find(v.begin(), v.end(), value) != v.end();
}

using ecstasy::MemoryPage;

#define TEST_MEMORY_LEAK_START MemoryPage::memoryLeakDetected = false; {
#define TEST_MEMORY_LEAK_END } if(MemoryPage::memoryLeakDetected) FAIL("Memory leak detected!");
