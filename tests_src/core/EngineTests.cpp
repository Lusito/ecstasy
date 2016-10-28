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
#include<limits>

#define NS_TEST_CASE(name) TEST_CASE("Engine: " name)
namespace EngineTests {
	const float deltaTime = 0.16f;
	struct ComponentA : public Component<ComponentA> {};
	struct ComponentB : public Component<ComponentB> {};
	struct ComponentC : public Component<ComponentC> {};

	class EntityListenerMock {
	public:
		int addedCount = 0;
		int removedCount = 0;

		void entityAdded (Entity* entity) {
			++addedCount;
			REQUIRE(entity);
		}

		void entityRemoved (Entity* entity) {
			++removedCount;
			REQUIRE(entity);
		}
	};

	struct MockLog {
		int updateCalls = 0;
		int addedCalls = 0;
		int removedCalls = 0;
	};

	template<typename T>
	class EntitySystemMockBase: public EntitySystem<T> {
	public:
		MockLog &log;
		std::vector<int>* updates = nullptr;

		EntitySystemMockBase(MockLog &log) : log(log) {}

		EntitySystemMockBase(MockLog &log, std::vector<int>* updates) : log(log), updates(updates) {}

		void update (float deltaTime) override {
			++log.updateCalls;

			if (updates != nullptr)
				updates->push_back(this->getPriority());
		}

		void addedToEngine (Engine* engine) override {
			++log.addedCalls;

			REQUIRE(engine);
		}

		void removedFromEngine (Engine* engine) override {
			++log.removedCalls;

			REQUIRE(engine);
		}
	};
	class EntitySystemMock : public EntitySystemMockBase<EntitySystemMock> {
	public:
		EntitySystemMock(MockLog &log) : EntitySystemMockBase(log) {}
	};

	class EntitySystemMockA : public EntitySystemMockBase<EntitySystemMockA> {
	public:
		EntitySystemMockA(MockLog &log) : EntitySystemMockBase(log) {}

		EntitySystemMockA(MockLog &log, std::vector<int>* updates) : EntitySystemMockBase(log, updates){}
	};

	class EntitySystemMockB : public EntitySystemMockBase<EntitySystemMockB> {
	public:
		EntitySystemMockB(MockLog &log) : EntitySystemMockBase(log) {}

		EntitySystemMockB(MockLog &log, std::vector<int>* updates) : EntitySystemMockBase(log, updates) {}
	};

	struct CounterComponent : public Component<CounterComponent> {
		int counter = 0;
	};

	class CounterSystem: public EntitySystem<CounterSystem> {
	public:
		const std::vector<Entity*  >* entities;
		Engine* engine;

		void addedToEngine (Engine* engine) override {
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

	struct PositionComponent : public Component<PositionComponent> {
		float x = 0.0f;
		float y = 0.0f;
	};

	class CombinedSystem : public EntitySystem<CombinedSystem> {
	public:
		Engine* engine;
		const std::vector<Entity*>* entities;
		int counter = 0;

	public:
		CombinedSystem(Engine* engine) : engine(engine) {}

		void addedToEngine(Engine* engine) override {
			entities = engine->getEntitiesFor(Family::all<PositionComponent>().get());
		}

		void update(float deltaTime) override {
			if (counter >= 6 && counter <= 8)
				engine->removeEntity(entities->at(2));
			counter++;
		}
	};

	class RemoveEntityTwiceSystem : public EntitySystem<RemoveEntityTwiceSystem> {
	private:
		const std::vector<Entity*>* entities;
		Engine* engine;

	public:
		RemoveEntityTwiceSystem() {}

		void addedToEngine(Engine* engine) override {
			entities = engine->getEntitiesFor(Family::all<PositionComponent>().get());
			this->engine = engine;
		}

		void update(float deltaTime) override {
			for (int i = 0; i < 10; i++) {
				auto entity = engine->createEntity();
				REQUIRE(0 == entity->flags);
				entity->flags = 1;
				entity->emplace<PositionComponent>();
				engine->addEntity(entity);
			}
			for (auto entity : *entities) {
				engine->removeEntity(entity);
				engine->removeEntity(entity);
			}
		}
	};

	NS_TEST_CASE("addAndRemoveEntity") {
		Engine engine;

		EntityListenerMock listenerA;
		EntityListenerMock listenerB;

		engine.entityAdded.connect(&listenerA, &EntityListenerMock::entityAdded);
		engine.entityRemoved.connect(&listenerA, &EntityListenerMock::entityRemoved);
		auto refBAdded = engine.entityAdded.connect(&listenerB, &EntityListenerMock::entityAdded);
		auto refBRemoved = engine.entityRemoved.connect(&listenerB, &EntityListenerMock::entityRemoved);

		Entity* entity1 = engine.createEntity();
		engine.addEntity(entity1);

		REQUIRE(1 == listenerA.addedCount);
		REQUIRE(1 == listenerB.addedCount);

		refBAdded.disable();
		refBRemoved.disable();

		Entity* entity2 = engine.createEntity();
		engine.addEntity(entity2);

		REQUIRE(2 == listenerA.addedCount);
		REQUIRE(1 == listenerB.addedCount);

		refBAdded.enable();
		refBRemoved.enable();

		engine.removeAllEntities();

		REQUIRE(2 == listenerA.removedCount);
		REQUIRE(2 == listenerB.removedCount);
	}

	NS_TEST_CASE("addAndRemoveSystem") {
		Engine engine;
		MockLog logA;
		MockLog logB;

		REQUIRE(!engine.getSystem<EntitySystemMockA>());
		REQUIRE(!engine.getSystem<EntitySystemMockB>());

		engine.emplaceSystem<EntitySystemMockA>(logA);
		engine.emplaceSystem<EntitySystemMockB>(logB);

		REQUIRE(engine.getSystem<EntitySystemMockA>());
		REQUIRE(engine.getSystem<EntitySystemMockB>());
		REQUIRE(1 == logA.addedCalls);
		REQUIRE(1 == logB.addedCalls);

		engine.removeSystem<EntitySystemMockA>();
		engine.removeSystem<EntitySystemMockB>();

		REQUIRE(!engine.getSystem<EntitySystemMockA>());
		REQUIRE(!engine.getSystem<EntitySystemMockB>());
		REQUIRE(1 == logA.removedCalls);
		REQUIRE(1 == logB.removedCalls);
	}

	NS_TEST_CASE("getSystems") {
		Engine engine;
		MockLog logA;
		MockLog logB;

		REQUIRE(engine.getSystems().empty());

		engine.emplaceSystem<EntitySystemMockA>(logA);
		engine.emplaceSystem<EntitySystemMockB>(logB);

		REQUIRE(2 == engine.getSystems().size());
	}

	NS_TEST_CASE("systemUpdate") {
		Engine engine;
		MockLog logA;
		MockLog logB;

		engine.emplaceSystem<EntitySystemMockA>(logA);
		engine.emplaceSystem<EntitySystemMockB>(logB);

		int numUpdates = 10;

		for (int i = 0; i < numUpdates; ++i) {
			REQUIRE(i == logA.updateCalls);
			REQUIRE(i == logB.updateCalls);

			engine.update(deltaTime);

			REQUIRE((i + 1) == logA.updateCalls);
			REQUIRE((i + 1) == logB.updateCalls);
		}

		engine.removeSystem<EntitySystemMockB>();

		for (int i = 0; i < numUpdates; ++i) {
			REQUIRE((i + numUpdates) == logA.updateCalls);
			REQUIRE(numUpdates == logB.updateCalls);

			engine.update(deltaTime);

			REQUIRE((i + 1 + numUpdates) == logA.updateCalls);
			REQUIRE(numUpdates == logB.updateCalls);
		}
	}
	NS_TEST_CASE("systemUpdateOrder") {
		std::vector<int> updates;

		Engine engine;
		MockLog log1;
		MockLog log2;

		engine.emplaceSystem<EntitySystemMockA>(log1, &updates)->setPriority(2);
		engine.emplaceSystem<EntitySystemMockB>(log2, &updates)->setPriority(1);

		engine.sortSystems();

		engine.update(deltaTime);

		int previous = std::numeric_limits<int>::min();

		for (int value : updates) {
			REQUIRE(value >= previous);
			previous = value;
		}
	}

	NS_TEST_CASE("ignoreSystem") {
		Engine engine;
		MockLog log;

		auto system = engine.emplaceSystem<EntitySystemMock>(log);

		int numUpdates = 10;

		for (int i = 0; i < numUpdates; ++i) {
			system->setProcessing(i % 2 == 0);
			engine.update(deltaTime);
			REQUIRE((i / 2 + 1) == log.updateCalls);
		}
	}

	NS_TEST_CASE("entitiesForFamily") {
		Engine engine;

		auto &family = Family::all<ComponentA, ComponentB>().get();
		auto familyEntities = engine.getEntitiesFor(family);

		REQUIRE(familyEntities->empty());

		Entity* entity1 = engine.createEntity();
		Entity* entity2 = engine.createEntity();
		Entity* entity3 = engine.createEntity();
		Entity* entity4 = engine.createEntity();

		entity1->emplace<ComponentA>();
		entity1->emplace<ComponentB>();

		entity2->emplace<ComponentA>();
		entity2->emplace<ComponentC>();

		entity3->emplace<ComponentA>();
		entity3->emplace<ComponentB>();
		entity3->emplace<ComponentC>();

		entity4->emplace<ComponentA>();
		entity4->emplace<ComponentB>();
		entity4->emplace<ComponentC>();

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

	NS_TEST_CASE("entityForFamilyWithRemoval") {
		// Test for issue #13
		Engine engine;

		Entity* entity = engine.createEntity();
		entity->emplace<ComponentA>();

		engine.addEntity(entity);

		auto entities = engine.getEntitiesFor(Family::all<ComponentA>().get());

		REQUIRE(1 == entities->size());
		REQUIRE(contains(*entities, entity));

		engine.removeEntity(entity);

		REQUIRE(entities->empty());
		REQUIRE(!contains(*entities, entity));
	}

	NS_TEST_CASE("entitiesForFamilyAfter") {
		Engine engine;

		auto &family = Family::all<ComponentA, ComponentB>().get();
		auto familyEntities = engine.getEntitiesFor(family);

		REQUIRE(familyEntities->empty());

		Entity* entity1 = engine.createEntity();
		Entity* entity2 = engine.createEntity();
		Entity* entity3 = engine.createEntity();
		Entity* entity4 = engine.createEntity();

		engine.addEntity(entity1);
		engine.addEntity(entity2);
		engine.addEntity(entity3);
		engine.addEntity(entity4);

		entity1->emplace<ComponentA>();
		entity1->emplace<ComponentB>();

		entity2->emplace<ComponentA>();
		entity2->emplace<ComponentC>();

		entity3->emplace<ComponentA>();
		entity3->emplace<ComponentB>();
		entity3->emplace<ComponentC>();

		entity4->emplace<ComponentA>();
		entity4->emplace<ComponentB>();
		entity4->emplace<ComponentC>();

		REQUIRE(3 == familyEntities->size());
		REQUIRE(contains(*familyEntities, entity1));
		REQUIRE(contains(*familyEntities, entity3));
		REQUIRE(contains(*familyEntities, entity4));
		REQUIRE(!contains(*familyEntities, entity2));
	}

	NS_TEST_CASE("entitiesForFamilyWithRemoval") {
		Engine engine;

		auto &family = Family::all<ComponentA, ComponentB>().get();
		auto familyEntities = engine.getEntitiesFor(family);

		Entity* entity1 = engine.createEntity();
		Entity* entity2 = engine.createEntity();
		Entity* entity3 = engine.createEntity();
		Entity* entity4 = engine.createEntity();

		engine.addEntity(entity1);
		engine.addEntity(entity2);
		engine.addEntity(entity3);
		engine.addEntity(entity4);

		entity1->emplace<ComponentA>();
		entity1->emplace<ComponentB>();

		entity2->emplace<ComponentA>();
		entity2->emplace<ComponentC>();

		entity3->emplace<ComponentA>();
		entity3->emplace<ComponentB>();
		entity3->emplace<ComponentC>();

		entity4->emplace<ComponentA>();
		entity4->emplace<ComponentB>();
		entity4->emplace<ComponentC>();

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

	NS_TEST_CASE("entitiesForFamilyWithRemovalAndFiltering") {
		Engine engine;

		auto entitiesWithComponentAOnly = engine.getEntitiesFor(Family::all<ComponentA>()
			.exclude<ComponentB>().get());

		auto entitiesWithComponentB = engine.getEntitiesFor(Family::all<ComponentB>().get());

		Entity* entity1 = engine.createEntity();
		Entity* entity2 = engine.createEntity();

		engine.addEntity(entity1);
		engine.addEntity(entity2);

		entity1->emplace<ComponentA>();

		entity2->emplace<ComponentA>();
		entity2->emplace<ComponentB>();

		REQUIRE(1 == entitiesWithComponentAOnly->size());
		REQUIRE(1 == entitiesWithComponentB->size());

		entity2->remove<ComponentB>();

		REQUIRE(2 == entitiesWithComponentAOnly->size());
		REQUIRE(entitiesWithComponentB->empty());
	}

	NS_TEST_CASE("entitySystemRemovalWhileIterating") {
		Engine engine;

		engine.emplaceSystem<CounterSystem>();

		for (int i = 0; i < 20; ++i) {
			Entity* entity = engine.createEntity();
			entity->emplace<CounterComponent>();
			engine.addEntity(entity);
		}

		auto entities = engine.getEntitiesFor(Family::all<CounterComponent>().get());

		for (auto e: *entities) {
			REQUIRE(0 == e->get<CounterComponent>()->counter);
		}

		engine.update(deltaTime);

		for (auto e : *entities) {
			REQUIRE(1 == e->get<CounterComponent>()->counter);
		}
	}

	NS_TEST_CASE("familyListener") {
		Engine engine;

		EntityListenerMock listenerA;
		EntityListenerMock listenerB;

		auto &familyA = Family::all<ComponentA>().get();
		auto &familyB = Family::all<ComponentB>().get();

		auto refAAdded = engine.getEntityAddedSignal(familyA).connect(&listenerA, &EntityListenerMock::entityAdded);
		auto refARemoved = engine.getEntityRemovedSignal(familyA).connect(&listenerA, &EntityListenerMock::entityRemoved);

		auto refBAdded = engine.getEntityAddedSignal(familyB).connect(&listenerB, &EntityListenerMock::entityAdded);
		auto refBRemoved = engine.getEntityRemovedSignal(familyB).connect(&listenerB, &EntityListenerMock::entityRemoved);

		Entity* entity1 = engine.createEntity();
		engine.addEntity(entity1);

		REQUIRE(0 == listenerA.addedCount);
		REQUIRE(0 == listenerB.addedCount);

		Entity* entity2 = engine.createEntity();
		engine.addEntity(entity2);

		REQUIRE(0 == listenerA.addedCount);
		REQUIRE(0 == listenerB.addedCount);

		entity1->emplace<ComponentA>();

		REQUIRE(1 == listenerA.addedCount);
		REQUIRE(0 == listenerB.addedCount);

		entity2->emplace<ComponentB>();

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
		entity2->emplace<ComponentB>();
		engine.addEntity(entity2);

		REQUIRE(1 == listenerA.addedCount);
		REQUIRE(1 == listenerB.addedCount);

		entity1->emplace<ComponentB>();
		entity1->emplace<ComponentA>();

		REQUIRE(2 == listenerA.addedCount);
		REQUIRE(1 == listenerB.addedCount);

		engine.removeAllEntities();

		REQUIRE(2 == listenerA.removedCount);
		REQUIRE(1 == listenerB.removedCount);

		refBAdded.enable();
		refBRemoved.enable();
	}

	NS_TEST_CASE("createManyEntitiesNoStackOverflow") {
		Engine engine;
		engine.emplaceSystem<CounterSystem>();

		for (int i = 0; 15000 > i; i++) {
			auto e = engine.createEntity();
			e->emplace<ComponentB>();
			engine.addEntity(e);
		}

		engine.update(0);
	}

	NS_TEST_CASE("getEntityById") {
		Engine engine;
		Entity* entity = engine.createEntity();

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

	NS_TEST_CASE("getEntities") {
		int numEntities = 10;

		Engine engine;

		std::vector<Entity*> entities;
		for (int i = 0; i < numEntities; ++i) {
			auto entity = engine.createEntity();
			entities.push_back(entity);
			engine.addEntity(entity);
		}

		const std::vector<Entity*>* engineEntities = engine.getEntities();

		REQUIRE(entities.size() == engineEntities->size());

		for (int i = 0; i < numEntities; ++i) {
			REQUIRE(entities.at(i) == engineEntities->at(i));
		}

		engine.removeAllEntities();

		REQUIRE(engineEntities->empty());
	}

	NS_TEST_CASE("addEntityTwice") {
		Engine engine;
		Entity* entity = engine.createEntity();
		engine.addEntity(entity);

		REQUIRE_THROWS_AS( engine.addEntity(entity), std::invalid_argument );
	}

	NS_TEST_CASE("addTwoSystemsOfSameClass") {
		Engine engine;
		MockLog log1;
		MockLog log2;

		REQUIRE(0 ==  engine.getSystems().size());
		auto system1 = engine.emplaceSystem<EntitySystemMockA>(log1);

		REQUIRE(1 == engine.getSystems().size());
		REQUIRE(system1 == engine.getSystem<EntitySystemMockA>());

		auto system2 = engine.emplaceSystem<EntitySystemMockA>(log2);

		REQUIRE(1 == engine.getSystems().size());
		REQUIRE(system2 == engine.getSystem<EntitySystemMockA>());
	}

	NS_TEST_CASE("entityRemovalListenerOrder") {
		Engine engine;

		auto combinedSystem = engine.emplaceSystem<CombinedSystem>(&engine);

		auto &signal = engine.getEntityRemovedSignal(Family::all<PositionComponent>().get());
		signal.connect([](Entity* entity) {
			REQUIRE(entity->get<PositionComponent>());
		});

		for (int i = 0; i < 10; i++) {
			auto entity = engine.createEntity();
			entity->emplace<PositionComponent>();
			engine.addEntity(entity);
		}

		REQUIRE(10 == combinedSystem->entities->size());

		float deltaTime = 0.16f;
		for (int i = 0; i < 10; i++)
			engine.update(deltaTime);

		engine.removeAllEntities();
	}

	NS_TEST_CASE("removeEntityTwice") {
		Engine engine;
		engine.emplaceSystem<RemoveEntityTwiceSystem>();

		for (int j = 0; j < 2; j++)
			engine.update(0);
	}

	NS_TEST_CASE("destroyEntity") {
		Engine engine;
		auto entity = engine.createEntity();
		engine.addEntity(entity);
		REQUIRE(entity->isValid());
		auto memoryManager = engine.getMemoryManager();
		REQUIRE(memoryManager->getAllocationCount() == 1);
		entity->destroy();
		REQUIRE(memoryManager->getAllocationCount() == 0);
	}

	NS_TEST_CASE("removeEntities") {
		Engine engine;

		int numEntities = 200;
		std::vector<Entity*> entities;

		for (int i = 0; i < numEntities; ++i) {
			auto entity = engine.createEntity();
			engine.addEntity(entity);
			entities.push_back(entity);

			REQUIRE(entity->isValid());
		}

		auto memoryManager = engine.getMemoryManager();
		REQUIRE(memoryManager->getAllocationCount() == numEntities);

		for (auto entity : entities) {
			engine.removeEntity(entity);
		}
		REQUIRE(memoryManager->getAllocationCount() == 0);
	}
}
