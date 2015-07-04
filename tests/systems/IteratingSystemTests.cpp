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
#include <ecstasy/systems/IteratingSystem.h>

namespace IteratingSystemTests {
	const float deltaTime = 0.16f;

	struct ComponentA : public Component<ComponentA> {};
	struct ComponentB : public Component<ComponentB> {};
	struct ComponentC : public Component<ComponentC> {};

	class IteratingSystemMock : public IteratingSystem<IteratingSystemMock>{
	public:
		int numUpdates = 0;

		IteratingSystemMock(const Family &family) : IteratingSystem(family){}

		void processEntity (Entity *entity, float deltaTime) override {
			++numUpdates;
		}
	};

	struct SpyComponent : public Component<SpyComponent> {
		int updates = 0;
	};

	struct IndexComponent : public Component<IndexComponent> {
		int index = 0;
	};

	class IteratingComponentRemovalSystem : public IteratingSystem<IteratingComponentRemovalSystem> {
	public:
		IteratingComponentRemovalSystem ()
			:IteratingSystem(Family::all<SpyComponent, IndexComponent>().get()) {}

		void processEntity (Entity *entity, float deltaTime) override {
			int index = entity->get<IndexComponent>()->index;
			if (index % 2 == 0) {
				entity->remove<SpyComponent>();
				entity->remove<IndexComponent>();
			} else {
				entity->get<SpyComponent>()->updates++;
			}
		}

	};

	class IteratingRemovalSystem : public IteratingSystem<IteratingRemovalSystem> {
	public:
		Engine *engine;

		IteratingRemovalSystem ()
			:IteratingSystem(Family::all<SpyComponent, IndexComponent>().get()) {}

		void addedToEngine(Engine *engine) override {
			IteratingSystem::addedToEngine(engine);
			this->engine = engine;
		}

		void processEntity(Entity *entity, float deltaTime) override {
			int index = entity->get<IndexComponent>()->index;
			if (index % 2 == 0)
				engine->removeEntity(entity);
			else
				entity->get<SpyComponent>()->updates++;
		}
	};

	TEST_CASE("shouldIterateEntitiesWithCorrectFamily") {
		Engine engine;

		auto &family = Family::all<ComponentA, ComponentB>().get();
		IteratingSystemMock system(family);
		Entity *e = engine.createEntity();

		engine.addSystem(&system);
		engine.addEntity(e);

		// When entity has ComponentA
		ComponentA *a = engine.createComponent<ComponentA>();
		e->add(a);
		engine.update(deltaTime);

		REQUIRE(0 == system.numUpdates);

		// When entity has ComponentA and ComponentB
		system.numUpdates = 0;
		ComponentB *b = engine.createComponent<ComponentB>();
		e->add(b);
		engine.update(deltaTime);

		REQUIRE(1 == system.numUpdates);

		// When entity has ComponentA, ComponentB and ComponentC
		system.numUpdates = 0;
		ComponentC *c = engine.createComponent<ComponentC>();
		e->add(c);
		engine.update(deltaTime);

		REQUIRE(1 == system.numUpdates);

		// When entity has ComponentB and ComponentC
		system.numUpdates = 0;
		e->remove<ComponentA>();
		engine.update(deltaTime);

		REQUIRE(0 == system.numUpdates);
		engine.clear();
	}

	TEST_CASE("entityRemovalWhileIterating") {
		Engine engine;
		auto *entities = engine.getEntitiesFor(Family::all<SpyComponent, IndexComponent>().get());

		IteratingRemovalSystem system;
		engine.addSystem(&system);

		int numEntities = 10;

		for (int i = 0; i < numEntities; ++i) {
			auto *e = engine.createEntity();
			e->add(engine.createComponent<SpyComponent>());

			auto *in = engine.createComponent<IndexComponent>();
			in->index = i + 1;

			e->add(in);

			engine.addEntity(e);
		}

		engine.update(deltaTime);

		REQUIRE((numEntities / 2) == entities->size());

		for (auto e : *entities) {
			REQUIRE(1 == e->get<SpyComponent>()->updates);
		}
		engine.clear();
	}

	TEST_CASE("componentRemovalWhileIterating") {
		Engine engine;
		auto *entities = engine.getEntitiesFor(Family::all<SpyComponent, IndexComponent>().get());

		IteratingComponentRemovalSystem system;
		engine.addSystem(&system);

		int numEntities = 10;

		for (int i = 0; i < numEntities; ++i) {
			auto *e = engine.createEntity();
			e->add(engine.createComponent<SpyComponent>());

			auto *in = engine.createComponent<IndexComponent>();
			in->index = i + 1;

			e->add(in);

			engine.addEntity(e);
		}

		engine.update(deltaTime);

		REQUIRE((numEntities / 2) == entities->size());

		for (auto e : *entities) {
			REQUIRE(1 == e->get<SpyComponent>()->updates);
		}
		engine.clear();
	}
}
