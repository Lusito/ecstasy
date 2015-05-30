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

namespace PooledEngineTests {
	struct PositionComponent : public Component<PositionComponent>, public Poolable {
		float x = 0.0f;
		float y = 0.0f;

		void reset() override {
			x = y = 0;
		}
	};

	class CombinedSystem : public EntitySystem<CombinedSystem> {
	public:
		Engine *engine;
		const std::vector<Entity *> *entities;
		int counter = 0;

	public:
		CombinedSystem(Engine *engine) : engine(engine) {}

		void addedToEngine(Engine *engine) override {
			entities = engine->getEntitiesFor(Family::all<PositionComponent>().get());
		}

		void update(float deltaTime) override {
			if (counter >= 6 && counter <= 8) {
				engine->removeEntity(entities->at(2));
			}
			counter++;
		}
	};

	class RemoveEntityTwiceSystem : public EntitySystem<RemoveEntityTwiceSystem> {
	private:
		const std::vector<Entity *> *entities;
		PooledEngine *engine;

	public:
		RemoveEntityTwiceSystem() {}

		void addedToEngine(Engine *engine) override {
			entities = engine->getEntitiesFor(Family::all<PositionComponent>().get());
			this->engine = (PooledEngine *)engine;
		}

		void update(float deltaTime) override {
			Entity *entity;
			for (int i = 0; i < 10; i++) {
				entity = engine->createEntity();
				REQUIRE(0 == entity->flags);
				entity->flags = 1;
				entity->add(engine->createComponent<PositionComponent>());
				engine->addEntity(entity);
			}
			for (Entity * entity : *entities) {
				engine->removeEntity(entity);
				engine->removeEntity(entity);
			}
		}
	};

	struct PooledComponentSpy : public Component<PooledComponentSpy>, public Poolable {
		bool recycled = false;

		void reset() override {
			recycled = true;
		}
	};


	TEST_CASE("entityRemovalListenerOrder") {
		PooledEngine engine;

		CombinedSystem combinedSystem(&engine);

		engine.addSystem(&combinedSystem);
		auto &signal = engine.getEntityRemovedSignal(Family::all<PositionComponent>().get());
		signal.connect([](Entity *entity) {
			PositionComponent *position = entity->get<PositionComponent>();
			REQUIRE(position);
		});

		for (int i = 0; i < 10; i++) {
			Entity *entity = engine.createEntity();
			entity->add(engine.createComponent<PositionComponent>());
			engine.addEntity(entity);
		}

		REQUIRE(10 == combinedSystem.entities->size());

		float deltaTime = 0.16f;
		for (int i = 0; i < 10; i++) {
			engine.update(deltaTime);
		}

		engine.removeAllEntities();
	}


	TEST_CASE("resetEntityCorrectly") {
		PooledEngine engine;

		// force the engine to create a Family so family bits get set
		const std::vector<Entity *> *familyEntities = engine.getEntitiesFor(Family::all<PositionComponent>().get());

		const int totalEntities = 10;
		Entity *entities[totalEntities];


		int totalAdds = 0;
		int totalRemoves = 0;
		engine.componentAdded.connect([&totalAdds](Entity *entity, ComponentBase *c) { totalAdds++; });
		engine.componentRemoved.connect([&totalRemoves](Entity *entity, ComponentBase *c) { totalRemoves++; });

		for (int i = 0; i < totalEntities; i++) {
			entities[i] = engine.createEntity();

			entities[i]->flags = 5;

			engine.addEntity(entities[i]);
			entities[i]->add(engine.createComponent<PositionComponent>());

//			REQUIRE(entities[i]->componentOperationHandler); //fixme
			REQUIRE(1 == entities[i]->getComponents().size());
			REQUIRE(!entities[i]->getFamilyBits().isEmpty());
			REQUIRE(contains(*familyEntities, entities[i]));
		}

		REQUIRE(totalEntities == totalAdds);
		REQUIRE(0 == totalRemoves);

		engine.removeAllEntities();

		REQUIRE(totalEntities == totalAdds);
		REQUIRE(totalEntities == totalRemoves);

		for (int i = 0; i < totalEntities; i++) {
			REQUIRE(0 == entities[i]->flags);
//			REQUIRE(!entities[i]->componentOperationHandler); //fixme
			REQUIRE(0 == entities[i]->getComponents().size());
			REQUIRE(entities[i]->getFamilyBits().isEmpty());
			REQUIRE(!contains(*familyEntities, entities[i]));
			REQUIRE(0L == entities[i]->getId());
		}
	}


	TEST_CASE("recycleEntity") {
		PooledEngine engine(10, 200, 10, 200);

		int numEntities = 200;
		std::vector<Entity *> entities;

		for (int i = 0; i < numEntities; ++i) {
			Entity *entity = engine.createEntity();
			engine.addEntity(entity);
			entities.push_back(entity);

			REQUIRE(0 != entity->getId());
		}

		int j = 0;
		for (Entity *entity : entities) {
			engine.removeEntity(entity);
			REQUIRE(0L == entity->getId());
			j++;
		}

		for (int i = 0; i < numEntities; ++i) {
			Entity *entity = engine.createEntity();
			engine.addEntity(entity);
			entities.push_back(entity);

			REQUIRE(0L != entity->getId());
		}
	}


	TEST_CASE("removeEntityTwice") {
		PooledEngine engine;
		RemoveEntityTwiceSystem system;
		engine.addSystem(&system);

		for (int j = 0; j < 2; j++) {
			engine.update(0);
		}
	}


	TEST_CASE("recycleComponent") {
		int maxEntities = 10;
		int maxComponents = 10;
		PooledEngine engine(maxEntities, maxEntities, maxComponents, maxComponents);

		for (int i = 0; i < maxComponents; ++i) {
			Entity *e = engine.createEntity();
			PooledComponentSpy *c = engine.createComponent<PooledComponentSpy>();

			REQUIRE(!c->recycled);

			e->add(c);

			engine.addEntity(e);
		}

		engine.removeAllEntities();

		for (int i = 0; i < maxComponents; ++i) {
			Entity *e = engine.createEntity();
			PooledComponentSpy *c = engine.createComponent<PooledComponentSpy>();

			REQUIRE(c->recycled);

			e->add(c);
		}

		engine.removeAllEntities();
	}
}
