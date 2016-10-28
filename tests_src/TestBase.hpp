#pragma once
#include "catch.hpp"
#include <vector>
#include <memory>
#include <ecstasy/core/Engine.hpp>
#include <ecstasy/core/EntitySystem.hpp>
#include <ecstasy/core/Family.hpp>
#include <ecstasy/core/Component.hpp>

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
