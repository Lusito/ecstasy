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

namespace EntityTests {
	struct ComponentA : public Component<ComponentA> {};
	struct ComponentB : public Component<ComponentB> {};

	TEST_CASE("uniqueIndex") {
		int numEntities = 10000;
		std::set<uint64_t> ids;
		Engine engine;

		for (int i = 0; i < numEntities; ++i) {
			auto entity = engine.createEntity();
			engine.addEntity(entity);
			REQUIRE(ids.insert(entity->getId()).second);
		}
	}


	TEST_CASE("noComponents") {
		Engine engine;
		Entity* entity = engine.createEntity();
		engine.addEntity(entity);

		REQUIRE(entity->getAll().empty());
		REQUIRE(entity->getComponentBits().isEmpty());
		REQUIRE(!entity->get<ComponentA>());
		REQUIRE(!entity->get<ComponentB>());
		REQUIRE(!entity->has<ComponentA>());
		REQUIRE(!entity->has<ComponentB>());
	}


	TEST_CASE("addAndRemoveComponent") {
		Engine engine;
		Entity* entity = engine.createEntity();
		engine.addEntity(entity);

		entity->emplace<ComponentA>();

		REQUIRE(1 == entity->getAll().size());

		const Bits &componentBits = entity->getComponentBits();
		auto componentAIndex = getComponentType<ComponentA>();
		
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
	}


	TEST_CASE("addAndRemoveAllComponents") {
		Engine engine;
		Entity* entity = engine.createEntity();
		engine.addEntity(entity);
		entity->emplace<ComponentA>();
		entity->emplace<ComponentB>();

		REQUIRE(2 == entity->getAll().size());

		auto &componentBits = entity->getComponentBits();
		auto componentAIndex = getComponentType<ComponentA>();
		auto componentBIndex = getComponentType<ComponentB>();
		
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
	}


	TEST_CASE("addSameComponent") {
		Engine engine;
		Entity* entity = engine.createEntity();
		engine.addEntity(entity);

		auto a1 = entity->emplace<ComponentA>();
		auto a2 = entity->emplace<ComponentA>();

		REQUIRE(1 == entity->getAll().size());
		REQUIRE(entity->has<ComponentA>());
		REQUIRE(a1 != entity->get<ComponentA>());
		REQUIRE(a2 == entity->get<ComponentA>());
	}


	TEST_CASE("componentListener") {
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
	}


	TEST_CASE("getComponentByClass") {
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
	}
}
