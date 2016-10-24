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

namespace EntityListenerTests {
	struct PositionComponent : public Component<PositionComponent> {};

	TEST_CASE("Add EntityListener Family Remove") {
		Engine engine;

		Entity *e = engine.createEntity();
		e->emplace<PositionComponent>();
		engine.addEntity(e);

		auto &signal = engine.getEntityRemovedSignal(Family::all<PositionComponent>().get());
		signal.connect([&](Entity *entity) {
			engine.addEntity(engine.createEntity());
		});

		engine.removeEntity(e);
	}

	TEST_CASE("addEntityListenerFamilyAdd") {
		Engine engine;

		Entity *e = engine.createEntity();
		e->emplace<PositionComponent>();

		auto &signal = engine.getEntityAddedSignal(Family::all<PositionComponent>().get());
		auto ref = signal.connect([&](Entity *entity) {
			engine.addEntity(engine.createEntity());
		});

		engine.addEntity(e);
		ref.disconnect();
		engine.removeAllEntities();
	}

	TEST_CASE("addEntityListenerNoFamilyRemove") {
		Engine engine;

		Entity *e = engine.createEntity();
		e->emplace<PositionComponent>();
		engine.addEntity(e);
		auto &family = Family::all<PositionComponent>().get();
		auto &signal = engine.getEntityRemovedSignal(family);
		auto ref = signal.connect([&](Entity *entity) {
			if (family.matches(entity))
				engine.addEntity(engine.createEntity());
		});

		engine.removeEntity(e);
		ref.disconnect();
	}

	TEST_CASE("addEntityListenerNoFamilyAdd") {
		Engine engine;

		Entity *e = engine.createEntity();
		e->emplace<PositionComponent>();

		auto &family = Family::all<PositionComponent>().get();
		auto &signal = engine.getEntityAddedSignal(family);
		signal.connect([&](Entity *entity) {
			if (family.matches(entity))
				engine.addEntity(engine.createEntity());
		});

		engine.addEntity(e);
	}

	class EntityRemoverSystem : public EntitySystem<EntityRemoverSystem> {
	public:
		EntityRemoverSystem(Entity *entity) : entity(entity) {}
		
		Entity *entity;
		
		void update(float deltaTime) override {
			getEngine()->removeEntity(entity);
		}
	};
	
	TEST_CASE("Remove entity during entity removal") {
		Engine engine;

		Entity *e1 = engine.createEntity();
		Entity *e2 = engine.createEntity();
		engine.addEntity(e1);
		engine.addEntity(e2);
		
		engine.emplaceSystem<EntityRemoverSystem>(e1);

		engine.entityRemoved.connect([&](Entity *entity) {
			if(entity == e1)
				engine.removeEntity(e2);
		});
		engine.update(0.16f);
	}
}
