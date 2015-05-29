/*******************************************************************************
 * Copyright 2014 See AUTHORS file.
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
#include "../TestBase.h"

namespace EntityTests {
	struct ComponentA : public Component<ComponentA> {};
	struct ComponentB : public Component<ComponentB> {};

	class EntityListenerMock : public Receiver < Entity * > {
	public:
		int counter = 0;

		void receive(Signal<Entity *> &signal, Entity *object) override {
			++counter;

			REQUIRE(object);
		}
	};

	TEST_CASE("uniqueIndex") {
		int numEntities = 10000;
		Allocator<Entity> entities;
		std::vector<uint64_t> ids;
		Engine engine;

		for (int i = 0; i < numEntities; ++i) {
			auto entity = entities.create();
			engine.addEntity(entity);
			REQUIRE(!contains(ids, entity->getId()));
			ids.push_back(entity->getId());
		}
	}


	TEST_CASE("noComponents") {
		Entity entity;

		REQUIRE(entity.getComponents().empty());
		//	REQUIRE(entity.getComponentBits().isEmpty()); // fixme
		REQUIRE(!entity.get<ComponentA>());
		REQUIRE(!entity.get<ComponentB>());
		REQUIRE(!entity.has<ComponentA>());
		REQUIRE(!entity.has<ComponentB>());
	}


	TEST_CASE("addAndRemoveComponent") {
		Entity entity;

		ComponentA a;
		entity.add(&a);

		REQUIRE(1 == entity.getComponents().size());

		//fixme:
		//	Bits &componentBits = entity.getComponentBits();
		//	int componentAIndex = ComponentType.getIndexFor(ComponentA.class);
		//
		//	for (int i = 0; i < componentBits.length(); ++i) {
		//		REQUIRE((i == componentAIndex) == componentBits.get(i));
		//	}

		REQUIRE(entity.get<ComponentA>());
		REQUIRE(!entity.get<ComponentB>());
		REQUIRE(entity.has<ComponentA>());
		REQUIRE(!entity.has<ComponentB>());

		entity.remove<ComponentA>();

		REQUIRE(0 == entity.getComponents().size());

		//fixme:
		//	for (int i = 0; i < componentBits.length(); ++i) {
		//		REQUIRE(!componentBits.get(i));
		//	}

		REQUIRE(!entity.get<ComponentA>());
		REQUIRE(!entity.get<ComponentB>());
		REQUIRE(!entity.has<ComponentA>());
		REQUIRE(!entity.has<ComponentB>());
	}


	TEST_CASE("addAndRemoveAllComponents") {
		Entity entity;
		ComponentA a;
		ComponentB b;
		entity.add(&a);
		entity.add(&b);

		REQUIRE(2 == entity.getComponents().size());

		//fixme:
		//	Bits &componentBits = entity.getComponentBits();
		//	ComponentType componentAIndex = getComponentType<ComponentA>();
		//	ComponentType componentBIndex = getComponentType<ComponentB>();
		//
		//	for (ComponentType i = 0; i < componentBits.length(); ++i) {
		//		REQUIRE((i == componentAIndex || i == componentBIndex) == componentBits.get(i));
		//	}

		REQUIRE(entity.get<ComponentA>());
		REQUIRE(entity.get<ComponentB>());
		REQUIRE(entity.has<ComponentA>());
		REQUIRE(entity.has<ComponentB>());

		entity.removeAll();

		REQUIRE(0 == entity.getComponents().size());

		//fixme:
		//	for (int i = 0; i < componentBits.length(); ++i) {
		//		REQUIRE(!componentBits.get(i));
		//	}

		REQUIRE(!entity.get<ComponentA>());
		REQUIRE(!entity.get<ComponentB>());
		REQUIRE(!entity.has<ComponentA>());
		REQUIRE(!entity.has<ComponentB>());
	}


	TEST_CASE("addSameComponent") {
		Entity entity;

		ComponentA a1;
		ComponentA a2;

		entity.add(&a1);
		entity.add(&a2);

		REQUIRE(1 == entity.getComponents().size());
		REQUIRE(entity.has<ComponentA>());
		REQUIRE(&a1 != entity.get<ComponentA>());
		REQUIRE(&a2 == entity.get<ComponentA>());
	}


	TEST_CASE("componentListener") {
		EntityListenerMock addedListener;
		EntityListenerMock removedListener;

		Entity entity;
		entity.componentAdded.add(&addedListener);
		entity.componentRemoved.add(&removedListener);

		REQUIRE(0 == addedListener.counter);
		REQUIRE(0 == removedListener.counter);

		ComponentA a;
		entity.add(&a);

		REQUIRE(1 == addedListener.counter);
		REQUIRE(0 == removedListener.counter);

		entity.remove<ComponentA>();

		REQUIRE(1 == addedListener.counter);
		REQUIRE(1 == removedListener.counter);

		ComponentB b;
		entity.add(&b);

		REQUIRE(2 == addedListener.counter);

		entity.remove<ComponentB>();

		REQUIRE(2 == removedListener.counter);
	}


	TEST_CASE("getComponentByClass") {
		ComponentA compA;
		ComponentB compB;

		Entity entity;
		entity.add(&compA).add(&compB);

		ComponentA *retA = entity.get<ComponentA>();
		ComponentB *retB = entity.get<ComponentB>();

		REQUIRE(retA);
		REQUIRE(retB);

		REQUIRE(retA == &compA);
		REQUIRE(retB == &compB);
	}
}