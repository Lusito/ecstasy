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
#include<set>

#define NS_TEST_CASE(name) TEST_CASE("Entity: " name)
namespace EntityTests {
	struct ComponentA : public Component<ComponentA> {};
	struct ComponentB : public Component<ComponentB> {};

	NS_TEST_CASE("uniqueIndex") {
		TEST_MEMORY_LEAK_START
		int numEntities = 10000;
		std::set<uint64_t> ids;
		Engine engine;

		for (int i = 0; i < numEntities; ++i) {
			auto entity = engine.createEntity();
			engine.addEntity(entity);
			REQUIRE(ids.insert(entity->getId()).second);
		}
		TEST_MEMORY_LEAK_END
	}

	NS_TEST_CASE("noComponents") {
		TEST_MEMORY_LEAK_START
		Engine engine;
		Entity* entity = engine.createEntity();
		engine.addEntity(entity);

		REQUIRE(entity->getAll().empty());
		REQUIRE(entity->getComponentBits().isEmpty());
		REQUIRE(!entity->get<ComponentA>());
		REQUIRE(!entity->get<ComponentB>());
		REQUIRE(!entity->has<ComponentA>());
		REQUIRE(!entity->has<ComponentB>());
		TEST_MEMORY_LEAK_END
	}

	NS_TEST_CASE("addAndRemoveComponent") {
		TEST_MEMORY_LEAK_START
		Engine engine;
		Entity* entity = engine.createEntity();
		engine.addEntity(entity);

		entity->emplace<ComponentA>();

		REQUIRE(1 == entity->getAll().size());

		const Bits &componentBits = entity->getComponentBits();
		auto componentAIndex = static_cast<int32_t>(getComponentType<ComponentA>());

		for (auto i = 0; i < componentBits.length(); ++i) {
			REQUIRE((i == componentAIndex) == componentBits.get(i));
		}

		REQUIRE(entity->get<ComponentA>());
		REQUIRE(!entity->get<ComponentB>());
		REQUIRE(entity->has<ComponentA>());
		REQUIRE(!entity->has<ComponentB>());

		entity->remove<ComponentA>();

		REQUIRE(0 == entity->getAll().size());

		for (int i = 0; i < componentBits.length(); ++i) {
			REQUIRE(!componentBits.get(i));
		}

		REQUIRE(!entity->get<ComponentA>());
		REQUIRE(!entity->get<ComponentB>());
		REQUIRE(!entity->has<ComponentA>());
		REQUIRE(!entity->has<ComponentB>());
		TEST_MEMORY_LEAK_END
	}

	NS_TEST_CASE("addAndRemoveAllComponents") {
		TEST_MEMORY_LEAK_START
		Engine engine;
		Entity* entity = engine.createEntity();
		engine.addEntity(entity);
		entity->emplace<ComponentA>();
		entity->emplace<ComponentB>();

		REQUIRE(2 == entity->getAll().size());

		auto &componentBits = entity->getComponentBits();
		auto componentAIndex = static_cast<int32_t>(getComponentType<ComponentA>());
		auto componentBIndex = static_cast<int32_t>(getComponentType<ComponentB>());

		for (auto i = 0; i < componentBits.length(); ++i) {
			REQUIRE((i == componentAIndex || i == componentBIndex) == componentBits.get(i));
		}

		REQUIRE(entity->get<ComponentA>());
		REQUIRE(entity->get<ComponentB>());
		REQUIRE(entity->has<ComponentA>());
		REQUIRE(entity->has<ComponentB>());

		entity->removeAll();

		REQUIRE(0 == entity->getAll().size());

		for (int i = 0; i < componentBits.length(); ++i) {
			REQUIRE(!componentBits.get(i));
		}

		REQUIRE(!entity->get<ComponentA>());
		REQUIRE(!entity->get<ComponentB>());
		REQUIRE(!entity->has<ComponentA>());
		REQUIRE(!entity->has<ComponentB>());
		TEST_MEMORY_LEAK_END
	}

	NS_TEST_CASE("addSameComponent") {
		TEST_MEMORY_LEAK_START
		Engine engine;
		Entity* entity = engine.createEntity();
		engine.addEntity(entity);

		auto a1 = entity->emplace<ComponentA>();
		auto a2 = entity->emplace<ComponentA>();

		REQUIRE(1 == entity->getAll().size());
		REQUIRE(entity->has<ComponentA>());
		REQUIRE(a1 != entity->get<ComponentA>());
		REQUIRE(a2 == entity->get<ComponentA>());
		TEST_MEMORY_LEAK_END
	}

	NS_TEST_CASE("componentListener") {
		TEST_MEMORY_LEAK_START
		Engine engine;
		Entity* entity = engine.createEntity();
		engine.addEntity(entity);

		int totalAdds = 0;
		int totalRemoves = 0;
		engine.componentAdded.connect([&totalAdds](Entity* e, ComponentBase* c) { totalAdds++; });
		engine.componentRemoved.connect([&totalRemoves](Entity* e, ComponentBase* c) { totalRemoves++; });

		REQUIRE(0 == totalAdds);
		REQUIRE(0 == totalRemoves);

		entity->emplace<ComponentA>();

		REQUIRE(1 == totalAdds);
		REQUIRE(0 == totalRemoves);

		entity->remove<ComponentA>();

		REQUIRE(1 == totalAdds);
		REQUIRE(1 == totalRemoves);

		entity->emplace<ComponentB>();

		REQUIRE(2 == totalAdds);

		entity->remove<ComponentB>();

		REQUIRE(2 == totalRemoves);
		TEST_MEMORY_LEAK_END
	}

	NS_TEST_CASE("getComponentByClass") {
		TEST_MEMORY_LEAK_START
		Engine engine;
		Entity* entity = engine.createEntity();
		engine.addEntity(entity);

		auto compA = entity->emplace<ComponentA>();
		auto compB = entity->emplace<ComponentB>();

		ComponentA* retA = entity->get<ComponentA>();
		ComponentB* retB = entity->get<ComponentB>();

		REQUIRE(retA);
		REQUIRE(retB);

		REQUIRE(retA == compA);
		REQUIRE(retB == compB);
		TEST_MEMORY_LEAK_END
	}
}
