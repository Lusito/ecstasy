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

namespace EntityListenerTests {
	struct PositionComponent : public Component<PositionComponent> {};

	class EntityListenerAddOnRemove : public EntityListener {
	public:
		Engine &engine;
		Allocator<Entity> entities;

	public:
		EntityListenerAddOnRemove(Engine &engine) : engine(engine) {}

		void entityRemoved(Entity *entity) override {
			engine.addEntity(entities.create());
		}

		void entityAdded(Entity *entity) override {

		}
	};
	TEST_CASE("Add EntityListener Family Remove") {
		Engine engine;

		std::shared_ptr<PositionComponent> component(new PositionComponent());
		Entity e;
		e.add(component.get());
		engine.addEntity(&e);

		auto &family = Family::all<PositionComponent>().get();
		EntityListenerAddOnRemove listener(engine);
		engine.addEntityListener(family, &listener);

		engine.removeEntity(&e);
		engine.clear();
	}


	class EntityListenerAddOnAdd : public EntityListener {
	public:
		Engine &engine;
		Allocator<Entity> entities;

	public:
		EntityListenerAddOnAdd(Engine &engine) : engine(engine) {}

		void entityRemoved(Entity *entity) override {
		}

		void entityAdded(Entity *entity) override {
			engine.addEntity(entities.create());
		}
	};
	TEST_CASE("addEntityListenerFamilyAdd") {
		Engine engine;

		Entity e;
		PositionComponent component;
		e.add(&component);

		auto &family = Family::all<PositionComponent>().get();
		EntityListenerAddOnAdd listener(engine);
		engine.addEntityListener(family, &listener);

		engine.addEntity(&e);
		engine.removeEntityListener(&listener);
		engine.removeAllEntities();
		engine.clear();
	}


	class EntityListenerAddOnRemoveIfFamilyMatches : public EntityListener {
	public:
		Engine &engine;
		Family &family;
		Allocator<Entity> entities;

	public:
		EntityListenerAddOnRemoveIfFamilyMatches(Engine &engine, Family &family) : engine(engine), family(family) {}

		void entityRemoved(Entity *entity) override {
			if (family.matches(entity)) engine.addEntity(entities.create());
		}

		void entityAdded(Entity *entity) override {
		}
	};
	TEST_CASE("addEntityListenerNoFamilyRemove") {
		Engine engine;

		Entity e;
		PositionComponent component;
		e.add(&component);
		engine.addEntity(&e);

		auto &family = Family::all<PositionComponent>().get();
		EntityListenerAddOnRemoveIfFamilyMatches listener(engine, family);
		engine.addEntityListener(&listener);

		engine.removeEntity(&e);
		engine.removeEntityListener(&listener);
		engine.clear();
	}

	class EntityListenerAddOnAddIfFamilyMatches : public EntityListener {
	public:
		Engine &engine;
		Family &family;
		Allocator<Entity> entities;

	public:
		EntityListenerAddOnAddIfFamilyMatches(Engine &engine, Family &family) : engine(engine), family(family) {}

		void entityRemoved(Entity *entity) override {
		}

		void entityAdded(Entity *entity) override {
			if (family.matches(entity)) engine.addEntity(entities.create());
		}
	};

	TEST_CASE("addEntityListenerNoFamilyAdd") {
		Engine engine;

		Entity e;
		PositionComponent component;
		e.add(&component);

		auto &family = Family::all<PositionComponent>().get();
		EntityListenerAddOnAddIfFamilyMatches listener(engine, family);
		engine.addEntityListener(&listener);

		engine.addEntity(&e);
		engine.clear();
	}
}