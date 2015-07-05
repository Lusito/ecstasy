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
#include<limits>

namespace EngineTests {
	const float deltaTime = 0.16f;
	struct ComponentA : public Component<ComponentA> {};
	struct ComponentB : public Component<ComponentB> {};
	struct ComponentC : public Component<ComponentC> {};

	class EntityListenerMock {
	public:
		int addedCount = 0;
		int removedCount = 0;

		void entityAdded (Entity *entity) {
			++addedCount;
			REQUIRE(entity);
		}

		void entityRemoved (Entity *entity) {
			++removedCount;
			REQUIRE(entity);
		}
	};

	template<typename T>
	class EntitySystemMockBase: public EntitySystem<T> {
	public:
		int updateCalls = 0;
		int addedCalls = 0;
		int removedCalls = 0;

		std::vector<int> *updates = nullptr;

		EntitySystemMockBase() {}
		
		EntitySystemMockBase(std::vector<int> *updates) : updates(updates) {}

		void update (float deltaTime) override {
			++updateCalls;

			if (updates != nullptr)
				updates->push_back(this->priority);
		}

		void addedToEngine (Engine *engine) override {
			++addedCalls;

			REQUIRE(engine);
		}

		void removedFromEngine (Engine *engine) override {
			++removedCalls;

			REQUIRE(engine);
		}
	};
	class EntitySystemMock : public EntitySystemMockBase<EntitySystemMock> {};

	class EntitySystemMockA : public EntitySystemMockBase<EntitySystemMockA> {
	public:
		EntitySystemMockA() {}

		EntitySystemMockA(std::vector<int> *updates) : EntitySystemMockBase(updates){}
	};

	class EntitySystemMockB : public EntitySystemMockBase<EntitySystemMockB> {
	public:
		EntitySystemMockB() {}

		EntitySystemMockB(std::vector<int> *updates) : EntitySystemMockBase(updates) {}
	};

	struct CounterComponent : public Component<CounterComponent> {
		int counter = 0;
	};

	class CounterSystem: public EntitySystem<CounterSystem> {
	public:
		const std::vector<Entity * > *entities;
		Engine *engine;

		void addedToEngine (Engine *engine) override {
			this->engine = engine;
			entities = engine->getEntitiesFor(Family::all<CounterComponent>().get());
		}

		void update (float deltaTime) override {
			for (size_t i = 0; i < entities->size(); ++i) {
				if (i % 2 == 0)
					entities->at(i)->get<CounterComponent>()->counter++;
				else
					engine->removeEntity(entities->at(i));
			}
		}
	};

	TEST_CASE("addAndRemoveEntity") {
		Engine engine;

		EntityListenerMock listenerA;
		EntityListenerMock listenerB;

		engine.entityAdded.connect(&listenerA, &EntityListenerMock::entityAdded);
		engine.entityRemoved.connect(&listenerA, &EntityListenerMock::entityRemoved);
		auto refBAdded = engine.entityAdded.connect(&listenerB, &EntityListenerMock::entityAdded);
		auto refBRemoved = engine.entityRemoved.connect(&listenerB, &EntityListenerMock::entityRemoved);

		Entity *entity1 = engine.createEntity();
		engine.addEntity(entity1);

		REQUIRE(1 == listenerA.addedCount);
		REQUIRE(1 == listenerB.addedCount);

		refBAdded.disable();
		refBRemoved.disable();

		Entity *entity2 = engine.createEntity();
		engine.addEntity(entity2);

		REQUIRE(2 == listenerA.addedCount);
		REQUIRE(1 == listenerB.addedCount);

		refBAdded.enable();
		refBRemoved.enable();

		engine.removeAllEntities();

		REQUIRE(2 == listenerA.removedCount);
		REQUIRE(2 == listenerB.removedCount);
	}

	TEST_CASE("addAndRemoveSystem") {
		Engine engine;
		EntitySystemMockA systemA;
		EntitySystemMockB systemB;

		REQUIRE(!engine.getSystem<EntitySystemMockA>());
		REQUIRE(!engine.getSystem<EntitySystemMockB>());

		engine.addSystem(&systemA);
		engine.addSystem(&systemB);

		REQUIRE(engine.getSystem<EntitySystemMockA>());
		REQUIRE(engine.getSystem<EntitySystemMockB>());
		REQUIRE(1 == systemA.addedCalls);
		REQUIRE(1 == systemB.addedCalls);

		engine.removeSystem(&systemA);
		engine.removeSystem(&systemB);

		REQUIRE(!engine.getSystem<EntitySystemMockA>());
		REQUIRE(!engine.getSystem<EntitySystemMockB>());
		REQUIRE(1 == systemA.removedCalls);
		REQUIRE(1 == systemB.removedCalls);
	}

	TEST_CASE("getSystems") {
		Engine engine;
		EntitySystemMockA systemA;
		EntitySystemMockB systemB;

		REQUIRE(engine.getSystems().empty());

		engine.addSystem(&systemA);
		engine.addSystem(&systemB);

		REQUIRE(2 == engine.getSystems().size());
	}

	TEST_CASE("systemUpdate") {
		Engine engine;
		EntitySystemMockA systemA;
		EntitySystemMockB systemB;

		engine.addSystem(&systemA);
		engine.addSystem(&systemB);

		int numUpdates = 10;

		for (int i = 0; i < numUpdates; ++i) {
			REQUIRE(i == systemA.updateCalls);
			REQUIRE(i == systemB.updateCalls);

			engine.update(deltaTime);

			REQUIRE((i + 1) == systemA.updateCalls);
			REQUIRE((i + 1) == systemB.updateCalls);
		}

		engine.removeSystem(&systemB);

		for (int i = 0; i < numUpdates; ++i) {
			REQUIRE((i + numUpdates) == systemA.updateCalls);
			REQUIRE(numUpdates == systemB.updateCalls);

			engine.update(deltaTime);

			REQUIRE((i + 1 + numUpdates) == systemA.updateCalls);
			REQUIRE(numUpdates == systemB.updateCalls);
		}
	}
	TEST_CASE("systemUpdateOrder") {
		std::vector<int> updates;

		Engine engine;
		EntitySystemMockA system1(&updates);
		EntitySystemMockB system2(&updates);

		system1.priority = 2;
		system2.priority = 1;

		engine.addSystem(&system1);
		engine.addSystem(&system2);

		engine.update(deltaTime);

		int previous = std::numeric_limits<int>::min();

		for (int value : updates) {
			REQUIRE(value >= previous);
			previous = value;
		}
	}

	TEST_CASE("ignoreSystem") {
		Engine engine;
		EntitySystemMock system;

		engine.addSystem(&system);

		int numUpdates = 10;

		for (int i = 0; i < numUpdates; ++i) {
			system.setProcessing(i % 2 == 0);
			engine.update(deltaTime);
			REQUIRE((i / 2 + 1) == system.updateCalls);
		}
	}

	TEST_CASE("entitiesForFamily") {
		Engine engine;

		auto &family = Family::all<ComponentA, ComponentB>().get();
		auto *familyEntities = engine.getEntitiesFor(family);

		REQUIRE(familyEntities->empty());

		Entity *entity1 = engine.createEntity();
		Entity *entity2 = engine.createEntity();
		Entity *entity3 = engine.createEntity();
		Entity *entity4 = engine.createEntity();
		
		entity1->add(engine.createComponent<ComponentA>());
		entity1->add(engine.createComponent<ComponentB>());

		entity2->add(engine.createComponent<ComponentA>());
		entity2->add(engine.createComponent<ComponentC>());

		entity3->add(engine.createComponent<ComponentA>());
		entity3->add(engine.createComponent<ComponentB>());
		entity3->add(engine.createComponent<ComponentC>());

		entity4->add(engine.createComponent<ComponentA>());
		entity4->add(engine.createComponent<ComponentB>());
		entity4->add(engine.createComponent<ComponentC>());

		engine.addEntity(entity1);
		engine.addEntity(entity2);
		engine.addEntity(entity3);
		engine.addEntity(entity4);

		REQUIRE(3 == familyEntities->size());
		REQUIRE(contains(*familyEntities, entity1));
		REQUIRE(contains(*familyEntities, entity3));
		REQUIRE(contains(*familyEntities, entity4));
		REQUIRE(!contains(*familyEntities, entity2));
	}

	TEST_CASE("entityForFamilyWithRemoval") {
		// Test for issue #13
		Engine engine;

		Entity *entity = engine.createEntity();
		ComponentA *a = engine.createComponent<ComponentA>();
		entity->add(a);

		engine.addEntity(entity);

		auto *entities = engine.getEntitiesFor(Family::all<ComponentA>().get());

		REQUIRE(1 == entities->size());
		REQUIRE(contains(*entities, entity));

		engine.removeEntity(entity);

		REQUIRE(entities->empty());
		REQUIRE(!contains(*entities, entity));
	}

	TEST_CASE("entitiesForFamilyAfter") {
		Engine engine;

		auto &family = Family::all<ComponentA, ComponentB>().get();
		auto *familyEntities = engine.getEntitiesFor(family);

		REQUIRE(familyEntities->empty());

		Entity *entity1 = engine.createEntity();
		Entity *entity2 = engine.createEntity();
		Entity *entity3 = engine.createEntity();
		Entity *entity4 = engine.createEntity();

		engine.addEntity(entity1);
		engine.addEntity(entity2);
		engine.addEntity(entity3);
		engine.addEntity(entity4);
		
		entity1->add(engine.createComponent<ComponentA>());
		entity1->add(engine.createComponent<ComponentB>());

		entity2->add(engine.createComponent<ComponentA>());
		entity2->add(engine.createComponent<ComponentC>());

		entity3->add(engine.createComponent<ComponentA>());
		entity3->add(engine.createComponent<ComponentB>());
		entity3->add(engine.createComponent<ComponentC>());

		entity4->add(engine.createComponent<ComponentA>());
		entity4->add(engine.createComponent<ComponentB>());
		entity4->add(engine.createComponent<ComponentC>());

		REQUIRE(3 == familyEntities->size());
		REQUIRE(contains(*familyEntities, entity1));
		REQUIRE(contains(*familyEntities, entity3));
		REQUIRE(contains(*familyEntities, entity4));
		REQUIRE(!contains(*familyEntities, entity2));
	}

	TEST_CASE("entitiesForFamilyWithRemoval") {
		Engine engine;

		auto &family = Family::all<ComponentA, ComponentB>().get();
		auto *familyEntities = engine.getEntitiesFor(family);

		Entity *entity1 = engine.createEntity();
		Entity *entity2 = engine.createEntity();
		Entity *entity3 = engine.createEntity();
		Entity *entity4 = engine.createEntity();

		engine.addEntity(entity1);
		engine.addEntity(entity2);
		engine.addEntity(entity3);
		engine.addEntity(entity4);

		entity1->add(engine.createComponent<ComponentA>());
		entity1->add(engine.createComponent<ComponentB>());

		entity2->add(engine.createComponent<ComponentA>());
		entity2->add(engine.createComponent<ComponentC>());

		entity3->add(engine.createComponent<ComponentA>());
		entity3->add(engine.createComponent<ComponentB>());
		entity3->add(engine.createComponent<ComponentC>());

		entity4->add(engine.createComponent<ComponentA>());
		entity4->add(engine.createComponent<ComponentB>());
		entity4->add(engine.createComponent<ComponentC>());

		REQUIRE(3 == familyEntities->size());
		REQUIRE(contains(*familyEntities, entity1));
		REQUIRE(contains(*familyEntities, entity3));
		REQUIRE(contains(*familyEntities, entity4));
		REQUIRE(!contains(*familyEntities, entity2));

		entity1->remove<ComponentA>();
		engine.removeEntity(entity3);

		REQUIRE(1 == familyEntities->size());
		REQUIRE(contains(*familyEntities, entity4));
		REQUIRE(!contains(*familyEntities, entity1));
		REQUIRE(!contains(*familyEntities, entity3));
		REQUIRE(!contains(*familyEntities, entity2));
	}

	TEST_CASE("entitiesForFamilyWithRemovalAndFiltering") {
		Engine engine;

		auto *entitiesWithComponentAOnly = engine.getEntitiesFor(Family::all<ComponentA>()
			.exclude<ComponentB>().get());

		auto *entitiesWithComponentB = engine.getEntitiesFor(Family::all<ComponentB>().get());

		Entity *entity1 = engine.createEntity();
		Entity *entity2 = engine.createEntity();

		engine.addEntity(entity1);
		engine.addEntity(entity2);

		entity1->add(engine.createComponent<ComponentA>());

		entity2->add(engine.createComponent<ComponentA>());
		entity2->add(engine.createComponent<ComponentB>());

		REQUIRE(1 == entitiesWithComponentAOnly->size());
		REQUIRE(1 == entitiesWithComponentB->size());

		entity2->remove<ComponentB>();

		REQUIRE(2 == entitiesWithComponentAOnly->size());
		REQUIRE(entitiesWithComponentB->empty());
	}

	TEST_CASE("entitySystemRemovalWhileIterating") {
		Engine engine;

		CounterSystem system;
		engine.addSystem(&system);

		for (int i = 0; i < 20; ++i) {
			Entity *entity = engine.createEntity();
			entity->add(engine.createComponent<CounterComponent>());
			engine.addEntity(entity);
		}

		auto *entities = engine.getEntitiesFor(Family::all<CounterComponent>().get());

		for (auto e: *entities) {
			REQUIRE(0 == e->get<CounterComponent>()->counter);
		}

		engine.update(deltaTime);

		for (auto e : *entities) {
			REQUIRE(1 == e->get<CounterComponent>()->counter);
		}
	}

	TEST_CASE("familyListener") {
		Engine engine;

		EntityListenerMock listenerA;
		EntityListenerMock listenerB;

		auto &familyA = Family::all<ComponentA>().get();
		auto &familyB = Family::all<ComponentB>().get();

		auto refAAdded = engine.getEntityAddedSignal(familyA).connect(&listenerA, &EntityListenerMock::entityAdded);
		auto refARemoved = engine.getEntityRemovedSignal(familyA).connect(&listenerA, &EntityListenerMock::entityRemoved);

		auto refBAdded = engine.getEntityAddedSignal(familyB).connect(&listenerB, &EntityListenerMock::entityAdded);
		auto refBRemoved = engine.getEntityRemovedSignal(familyB).connect(&listenerB, &EntityListenerMock::entityRemoved);

		Entity *entity1 = engine.createEntity();
		engine.addEntity(entity1);

		REQUIRE(0 == listenerA.addedCount);
		REQUIRE(0 == listenerB.addedCount);

		Entity *entity2 = engine.createEntity();
		engine.addEntity(entity2);

		REQUIRE(0 == listenerA.addedCount);
		REQUIRE(0 == listenerB.addedCount);
		
		entity1->add(engine.createComponent<ComponentA>());

		REQUIRE(1 == listenerA.addedCount);
		REQUIRE(0 == listenerB.addedCount);

		entity2->add(engine.createComponent<ComponentB>());

		REQUIRE(1 == listenerA.addedCount);
		REQUIRE(1 == listenerB.addedCount);

		entity1->remove<ComponentA>();

		REQUIRE(1 == listenerA.removedCount);
		REQUIRE(0 == listenerB.removedCount);

		engine.removeEntity(entity2);

		REQUIRE(1 == listenerA.removedCount);
		REQUIRE(1 == listenerB.removedCount);

		refBAdded.disable();
		refBRemoved.disable();

		entity2 = engine.createEntity();
		entity2->add(engine.createComponent<ComponentB>());
		engine.addEntity(entity2);

		REQUIRE(1 == listenerA.addedCount);
		REQUIRE(1 == listenerB.addedCount);

		entity1->add(engine.createComponent<ComponentB>());
		entity1->add(engine.createComponent<ComponentA>());

		REQUIRE(2 == listenerA.addedCount);
		REQUIRE(1 == listenerB.addedCount);

		engine.removeAllEntities();

		REQUIRE(2 == listenerA.removedCount);
		REQUIRE(1 == listenerB.removedCount);

		refBAdded.enable();
		refBRemoved.enable();
	}

	TEST_CASE("createManyEntitiesNoStackOverflow") {
		Engine engine;
		CounterSystem system;
		engine.addSystem(&system);

		for (int i = 0; 15000 > i; i++) {
			auto *e = engine.createEntity();
			e->add(engine.createComponent<ComponentB>());
			engine.addEntity(e);
		}

		engine.update(0);
	}
	
	TEST_CASE("getEntityById") {
		Engine engine;
		Entity *entity = engine.createEntity();
		
		REQUIRE(0 == entity->getId());
		REQUIRE(!entity->isValid());
		
		engine.addEntity(entity);
		
		REQUIRE(entity->isValid());
		
		uint64_t entityId = entity->getId();
		
		REQUIRE(0 != entityId);
		
		REQUIRE(entity == engine.getEntity(entityId));
		
		engine.removeEntity(entity);
		
		REQUIRE(!engine.getEntity(entityId));
	}
	
	TEST_CASE("getEntities") {
		int numEntities = 10;
		
		Engine engine;
		
		std::vector<Entity *> entities;
		for (int i = 0; i < numEntities; ++i) {
			auto *entity = engine.createEntity();
			entities.push_back(entity);
			engine.addEntity(entity);
		}
		
		const std::vector<Entity*> *engineEntities = engine.getEntities();
		
		REQUIRE(entities.size() == engineEntities->size());
		
		for (int i = 0; i < numEntities; ++i) {
			REQUIRE(entities.at(i) == engineEntities->at(i));
		}
		
		engine.removeAllEntities();
		
		REQUIRE(engineEntities->empty());
	}
	
	TEST_CASE("addEntityTwice") {
		Engine engine;
		Entity *entity = engine.createEntity();
		engine.addEntity(entity);

		REQUIRE_THROWS_AS( engine.addEntity(entity), std::invalid_argument );
	}
	
	
	TEST_CASE("addTwoSystemsOfSameClass") {
		Engine engine;
		EntitySystemMockA system1;
		EntitySystemMockA system2;

		REQUIRE(0 ==  engine.getSystems().size());
		engine.addSystem(&system1);

		REQUIRE(1 == engine.getSystems().size());
		REQUIRE(&system1 == engine.getSystem<EntitySystemMockA>());

		engine.addSystem(&system2);

		REQUIRE(1 == engine.getSystems().size());
		REQUIRE(&system2 == engine.getSystem<EntitySystemMockA>());
	}
}
