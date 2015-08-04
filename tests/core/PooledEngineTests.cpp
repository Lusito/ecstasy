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
#include "../TestBase.h"

namespace PooledEngineTests {
	struct PositionComponent : public Component<PositionComponent> {
		float x = 0.0f;
		float y = 0.0f;
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
			if (counter >= 6 && counter <= 8)
				engine->removeEntity(entities->at(2));
			counter++;
		}
	};

	class RemoveEntityTwiceSystem : public EntitySystem<RemoveEntityTwiceSystem> {
	private:
		const std::vector<Entity *> *entities;
		Engine *engine;

	public:
		RemoveEntityTwiceSystem() {}

		void addedToEngine(Engine *engine) override {
			entities = engine->getEntitiesFor(Family::all<PositionComponent>().get());
			this->engine = engine;
		}

		void update(float deltaTime) override {
			for (int i = 0; i < 10; i++) {
				auto *entity = engine->createEntity();
				REQUIRE(0 == entity->flags);
				entity->flags = 1;
				entity->add(engine->createComponent<PositionComponent>());
				engine->addEntity(entity);
			}
			for (auto * entity : *entities) {
				engine->removeEntity(entity);
				engine->removeEntity(entity);
			}
		}
	};

	TEST_CASE("entityRemovalListenerOrder") {
		Engine engine;

		CombinedSystem combinedSystem(&engine);

		engine.addSystem(&combinedSystem);
		auto &signal = engine.getEntityRemovedSignal(Family::all<PositionComponent>().get());
		signal.connect([](Entity *entity) {
			REQUIRE(entity->get<PositionComponent>());
		});

		for (int i = 0; i < 10; i++) {
			auto *entity = engine.createEntity();
			entity->add(engine.createComponent<PositionComponent>());
			engine.addEntity(entity);
		}

		REQUIRE(10 == combinedSystem.entities->size());

		float deltaTime = 0.16f;
		for (int i = 0; i < 10; i++)
			engine.update(deltaTime);

		engine.removeAllEntities();
	}


	TEST_CASE("resetEntityCorrectly") {
		Engine engine;

		// force the engine to create a Family so family bits get set
		auto *familyEntities = engine.getEntitiesFor(Family::all<PositionComponent>().get());

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

			REQUIRE(1 == entities[i]->getAll().size());
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
			REQUIRE(0 == entities[i]->getAll().size());
			REQUIRE(entities[i]->getFamilyBits().isEmpty());
			REQUIRE(!contains(*familyEntities, entities[i]));
			REQUIRE(!entities[i]->isValid());
		}
	}


	TEST_CASE("recycleEntity") {
		Engine engine(10, 200);

		int numEntities = 200;
		std::vector<Entity *> entities;

		for (int i = 0; i < numEntities; ++i) {
			auto *entity = engine.createEntity();
			engine.addEntity(entity);
			entities.push_back(entity);

			REQUIRE(entity->isValid());
		}

		int j = 0;
		for (auto *entity : entities) {
			engine.removeEntity(entity);
			REQUIRE(!entity->isValid());
			j++;
		}

		for (int i = 0; i < numEntities; ++i) {
			auto *entity = engine.createEntity();
			engine.addEntity(entity);
			entities.push_back(entity);

			REQUIRE(entity->isValid());
		}
	}


	TEST_CASE("removeEntityTwice") {
		Engine engine;
		RemoveEntityTwiceSystem system;
		engine.addSystem(&system);

		for (int j = 0; j < 2; j++)
			engine.update(0);
	}
}
