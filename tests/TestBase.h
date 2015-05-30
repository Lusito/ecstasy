#pragma once
#include "catch.hpp"
#include <vector>
#include <memory>
#include <ecstasy/core/Engine.h>
#include <ecstasy/core/PooledEngine.h>
#include <ecstasy/core/Component.h>
#include <ecstasy/core/EntityListener.h>

using namespace ECS;

template<typename T>
class Allocator {
public:
	std::vector<std::shared_ptr<T>> values;
	
	T *create() {
		auto instance = std::shared_ptr<T>(new T());
		values.push_back(instance);
		return instance.get();
	}
};
template<typename T>
bool contains(const std::vector<T> &v, const T &value) {
	return std::find(v.begin(), v.end(), value) != v.end();
}