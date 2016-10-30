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

#define NS_TEST_CASE(name) TEST_CASE("ComponentType: " name)
namespace ComponentTypeTests {
	struct ComponentA : public Component<ComponentA> {};
	struct ComponentB : public Component<ComponentB> {};

	NS_TEST_CASE("sameComponentType") {
		auto componentType1 = getComponentType<ComponentA>();
		auto componentType2 = getComponentType<ComponentA>();

		REQUIRE(componentType1 == componentType2);
	}

	NS_TEST_CASE("differentComponentType") {
		auto componentType1 = getComponentType<ComponentA>();
		auto componentType2 = getComponentType<ComponentB>();

		REQUIRE(componentType1 != componentType2);
	}
}
